#include <iostream>
#include <vector>
#include <omp.h>

int Head { 4 };
std::vector<int> Next = { 5, -1, 0, 1,  2, 3, -1, -1, -1, -1, -1 };
std::vector<int> Prev = { 2,  3, 4, 5, -1, 0, -1, -1, -1, -1, -1 };
std::vector<omp_lock_t> Locks(11);

omp_lock_t ListLock;

void Init_locks()
{
  omp_init_lock( &ListLock );
  
  for ( unsigned i = 0 ; i < Locks.size() ; ++i )
  {
    omp_init_lock( &Locks[i] );
  }
}

void Destroy_locks()
{
  omp_destroy_lock( &ListLock );
  
  for ( unsigned i = 0 ; i < Locks.size() ; ++i )
  {
    omp_destroy_lock( &Locks[i] );
  }
}

void Print()
{
  int curr { Head };
  
  std::cout << curr;
  while ( curr != -1 )
  {
    curr = Next[curr];
    std::cout << " -> " << curr;
  }
}

bool Validate( const int& prev, const int& curr )
{
  if ( prev == -1 ) return Head == curr;
  if ( curr == -1 ) return Next[prev] == curr;
  
  return Next[prev] == curr && Prev[curr] == prev;
}

void Remove( const int& Id )
{
  while ( true )
  {
    // Connection info *********************************************************
    int prev { Prev[Id] };
    int next { Next[Id] };
    
    // Lock associated locks ***************************************************
    ( prev != -1 ) ? omp_set_lock( &Locks[prev] ) : omp_set_lock( &ListLock );
    
    omp_set_lock( &Locks[Id]   );
    
    if ( next != -1 ) omp_set_lock( &Locks[next] );
    
    // Do validation ***********************************************************
    if ( !Validate( prev, Id ) || !Validate( Id, next ) )
    {
      ( prev != -1 ) ? omp_unset_lock( &Locks[prev] ) : omp_unset_lock( &ListLock );
    
      omp_unset_lock( &Locks[Id]   );
    
      if ( next != -1 ) omp_unset_lock( &Locks[next] );
      
      continue;
    }
    
    // Remove the target *******************************************************
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
    
    // Unlock the locks ********************************************************
    ( prev != -1 ) ? omp_unset_lock( &Locks[prev] ) : omp_unset_lock( &ListLock );
    
    omp_unset_lock( &Locks[Id]   );
    
    if ( next != -1 ) omp_unset_lock( &Locks[next] );
    
    break;
  }
}

void AddFront( const int& Id )
{
  while (true)
  {
    int head { Head };
    
    if ( head != -1 ) omp_set_lock( &Locks[head] );
    omp_set_lock( &ListLock );
    omp_set_lock( &Locks[Id] );
    
    // Do validation ***********************************************************
    if ( Head != head )
    {
      if ( head != -1 ) omp_unset_lock( &Locks[head] );
      omp_unset_lock( &ListLock );
      omp_unset_lock( &Locks[Id] );
      
      continue;
    }
    
    // Add the target to the list **********************************************
    if ( head != -1 )
    {
      Prev[head] = Id;
    }
    
    Next[Id] = head;
    Prev[Id] = -1;
    
    Head = Id;
    
    // Unlock associated locks *************************************************
    if ( head != -1 ) omp_unset_lock( &Locks[head] );
    omp_unset_lock( &ListLock );
    omp_unset_lock( &Locks[Id] );
    
    break;
  }
}

int main()
{
  Print();
  
  std::vector<int> RemoveId = { 3, 1, 2, 4, 0 };
  std::vector<int> AddId = { 6, 7, 8, 9, 10 };
  
  std::vector<int> Id = { 6, 9, -3, 7, -4, -2, 10, -1, 8 };
  
  Init_locks();
  
  #pragma omp parallel
  {
    //#pragma omp for
    //for ( unsigned i = 0 ; i < RemoveId.size() ; ++i )
    //  Remove( RemoveId[i] );
    
    //#pragma omp for
    //for ( unsigned i = 0 ; i < AddId.size() ; ++i )
    //  AddFront( AddId[i] );
    
    #pragma omp for
    for ( unsigned i = 0 ; i < Id.size() ; ++i )
      ( Id[i] < 0 ) ? Remove( -Id[i] ) : AddFront( Id[i] );
  }
  
  Destroy_locks();
  
  std::cout << '\n';
  Print();
  
  return 0;
}