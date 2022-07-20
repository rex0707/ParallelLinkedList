/*********************************************************************************************
 * This example implements the optimistic locking doubly linked list. The original list is
 * 4 -> 2 -> 0 -> 5 -> 3 -> 1 -> -1
 *
 * The Head is 4 and connection information are stored in the vectors Next and Prev.
 * Next[4] = 2, Next[2] = 0, Next[0] = 5, and so on.
 * Prev[1] = 3, Prev[3] = 5, Prev[5] = 0, and so on.
 *
 * For testing the algorithm is correct, a vector Id is defined in which the positive numbers 
 * are items to be added to the front of the list and the negative numbers are the items to be
 * removed from the list.
 *
 * Ex: std::vector<int> Id = { 6, 9, -3, 7, -4, -2, 10, -1, 8 };
 *     Items 3, 4, 2, and 1 will be removed from the original list, and
 *     items 6, 9, 7, 10 and 8 will be added to the front of the list.
 *
 * Type omp_lock_t is used to lock and unlock the items and the head of the list.
 *
 * The final result may be
 * 6 -> 7 -> 8 -> 9 -> 10 -> 0 -> 5 -> -1
 * +    +    +    +     +
 *
 * The order of the nodes with mark + can be different because different threads may execute
 * the AddFront function at different times.
 ********************************************************************************************/

#include <iostream>
#include <vector>
#include <omp.h>

/*********************************************************************************************
 * Global variables
 ********************************************************************************************/
int Head { 4 };
std::vector<int> Next = { 5, -1, 0, 1,  2, 3, -1, -1, -1, -1, -1 };
std::vector<int> Prev = { 2,  3, 4, 5, -1, 0, -1, -1, -1, -1, -1 };

std::vector<omp_lock_t> Locks(11);
omp_lock_t ListLock;

/*********************************************************************************************
 * Forward declarations
 ********************************************************************************************/
void Init_locks();

void Destroy_locks();

void Print();

bool Validate( const int& prev, const int& curr );

void Remove( const int& Id );

void AddFront( const int& Id );

/*********************************************************************************************
 * main function
 ********************************************************************************************/
int main()
{
  // Print the original list ************************************************************
  Print();
  
  // 
  std::vector<int> Id = { 6, 9, -3, 7, -4, -2, 10, -1, 8 };
  
  // Initialize the locks ***************************************************************
  Init_locks();
  
  #pragma omp parallel for
  for ( unsigned i = 0 ; i < Id.size() ; ++i )
    ( Id[i] < 0 ) ? Remove( -Id[i] ) : AddFront( Id[i] );
  
  // Destroy the locks ******************************************************************
  Destroy_locks();
  
  // Print the final list ***************************************************************
  Print();
  
  return 0;
}
/*********************************************************************************************
 * Initialize omp_lock_t objects
 ********************************************************************************************/
void Init_locks()
{
  omp_init_lock( &ListLock );
  
  for ( unsigned i = 0 ; i < Locks.size() ; ++i )
  {
    omp_init_lock( &Locks[i] );
  }
}
/*********************************************************************************************
 * Destroy omp_lock_t objects
 ********************************************************************************************/
void Destroy_locks()
{
  omp_destroy_lock( &ListLock );
  
  for ( unsigned i = 0 ; i < Locks.size() ; ++i )
  {
    omp_destroy_lock( &Locks[i] );
  }
}
/*********************************************************************************************
 * Print the list
 ********************************************************************************************/
void Print()
{
  std::cout << std::endl;
  
  int curr { Head };
  
  std::cout << curr;
  while ( curr != -1 )
  {
    curr = Next[curr];
    std::cout << " -> " << curr;
  }
  
  std::cout << std::endl;
}
/*********************************************************************************************
 * Validate two consecutive items
 ********************************************************************************************/
bool Validate( const int& prev, const int& curr )
{
  if ( prev == -1 ) return Head == curr;
  if ( curr == -1 ) return Next[prev] == curr;
  
  return Next[prev] == curr && Prev[curr] == prev;
}
/*********************************************************************************************
 * Remove item Id
 ********************************************************************************************/
void Remove( const int& Id )
{
  while ( true )
  {
    // Connection info *************************************************************
    int prev { Prev[Id] };
    int next { Next[Id] };
    
    // Lock associated locks *******************************************************
    ( prev != -1 ) ? omp_set_lock( &Locks[prev] ) : omp_set_lock( &ListLock );
    
    omp_set_lock( &Locks[Id]   );
    
    if ( next != -1 ) omp_set_lock( &Locks[next] );
    
    // Do validation ***************************************************************
    if ( !Validate( prev, Id ) || !Validate( Id, next ) )
    {
      ( prev != -1 ) ? omp_unset_lock( &Locks[prev] ) : omp_unset_lock( &ListLock );
    
      omp_unset_lock( &Locks[Id]   );
    
      if ( next != -1 ) omp_unset_lock( &Locks[next] );
      
      continue;
    }
    
    // Remove the target ***********************************************************
    if ( prev == -1 )
    {
      Head = next;
    }
    else
    {
      Next[prev] = next;
    }
    
    if ( next != -1 )
    {
      Prev[next] = prev;
    }
    
    // Unlock the locks ************************************************************
    ( prev != -1 ) ? omp_unset_lock( &Locks[prev] ) : omp_unset_lock( &ListLock );
    
    omp_unset_lock( &Locks[Id]   );
    
    if ( next != -1 ) omp_unset_lock( &Locks[next] );
    
    break;
  }
}
/*********************************************************************************************
 * Add item Id to the front of the list
 ********************************************************************************************/
void AddFront( const int& Id )
{
  while (true)
  {
    // Connection info *************************************************************
    int head { Head };
    
    if ( head != -1 ) omp_set_lock( &Locks[head] );
    omp_set_lock( &ListLock );
    omp_set_lock( &Locks[Id] );
    
    // Do validation ***************************************************************
    if ( Head != head )
    {
      if ( head != -1 ) omp_unset_lock( &Locks[head] );
      omp_unset_lock( &ListLock );
      omp_unset_lock( &Locks[Id] );
      
      continue;
    }
    
    // Add the target to the list **************************************************
    if ( head != -1 )
    {
      Prev[head] = Id;
    }
    
    Next[Id] = head;
    Prev[Id] = -1;
    
    Head = Id;
    
    // Unlock associated locks *****************************************************
    if ( head != -1 ) omp_unset_lock( &Locks[head] );
    omp_unset_lock( &ListLock );
    omp_unset_lock( &Locks[Id] );
    
    break;
  }
}