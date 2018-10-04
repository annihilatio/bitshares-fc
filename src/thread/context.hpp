#pragma once
#include <fc/thread/thread.hpp>
//#include <boost/context/all.hpp>
#include <boost/context/execution_context.hpp>
#include <fc/exception/exception.hpp>
#include <vector>

#include <fc/thread/scoped_lock.hpp>
#include <fc/thread/spin_lock.hpp>

#include <boost/version.hpp>

# include <boost/coroutine/stack_context.hpp>
  namespace bc  = boost::context;
  namespace bco = boost::coroutines;
# if defined(NDEBUG)
#  include <boost/coroutine/stack_allocator.hpp>
  typedef bco::stack_allocator stack_allocator;
#else
#  include <boost/assert.hpp>
#  include <boost/coroutine/protected_stack_allocator.hpp>
  typedef bco::protected_stack_allocator stack_allocator;
# endif

namespace boost { namespace context { namespace detail {
    struct transfer_t;
} } }

namespace fc {
  class thread;
  class promise_base;
  class task_base;

  /**
   *  maintains information associated with each context such as
   *  where it is blocked, what time it should resume, priority,
   *  etc.
   */
  
  class context_counter
  {
    context_counter(): m_count(0){}
    ~context_counter(){}
    uint32_t count_impl()
    {
      scoped_lock<spin_lock> lock(m_lock);
      return ++m_count;
    }
    uint32_t m_count;
    spin_lock m_lock;
  public:
    static uint32_t count()
    {
      static context_counter counter;
      return counter.count_impl();
    }
  };
  
  struct context  {
    typedef fc::context* ptr;

    uint32_t context_id;
    bco::stack_context stack_ctx;


    context( void (*sf)(boost::context::detail::transfer_t), stack_allocator& alloc, fc::thread* t )
    : context_id(context_counter::count()),
      caller_context(0),
      stack_alloc(&alloc),
      next_blocked(0), 
      next_blocked_mutex(0), 
      next(0), 
      ctx_thread(t),
      canceled(false),
#ifndef NDEBUG
      cancellation_reason(nullptr),
#endif
      complete(false),
      cur_task(0),
      context_posted_num(0)
    {
     size_t stack_size = FC_CONTEXT_STACK_SIZE;
     alloc.allocate(stack_ctx, stack_size);
     my_context = bc::detail::make_fcontext( stack_ctx.sp, stack_ctx.size, sf);
     // DDLOG("Created fc::context at %p with my_context=%p", this, my_context);
    }

    context( fc::thread* t) :
      context_id(0),
      my_context(nullptr),
      caller_context(0),
      stack_alloc(0),
      next_blocked(0),
      next_blocked_mutex(0),
      next(0),
      ctx_thread(t),
      canceled(false),
#ifndef NDEBUG
      cancellation_reason(nullptr),
#endif
      complete(false),
      cur_task(0),
      context_posted_num(0)
    { /* DDLOG("Created fc::context at %p with NULL my_context", this); */ }

    ~context() {
      // DDLOG("Destroyed fc::context at %p", this);
      if(stack_alloc)
        stack_alloc->deallocate( stack_ctx );
    }

    void reinitialize()
    {
      canceled = false;
#ifndef NDEBUG
      cancellation_reason = nullptr;
#endif
      blocking_prom.clear();
      caller_context = nullptr;
      resume_time = fc::time_point();
      next_blocked = nullptr;
      next_blocked_mutex = nullptr;
      next = nullptr;
      complete = false;
    }

    struct blocked_promise {
      blocked_promise( promise_base* p=0, bool r=true )
      :prom(p),required(r){}

      promise_base* prom;
      bool          required;
    };
    
    /**
     *  @todo Have a list of promises so that we can wait for
     *    P1 or P2 and either will unblock instead of requiring both
     *  @param req - require this promise to 'unblock', otherwise try_unblock
     *     will allow it to be one of many that could 'unblock'
     */
    void add_blocking_promise( promise_base* p, bool req = true ) {
      for( auto i = blocking_prom.begin(); i != blocking_prom.end(); ++i ) {
        if( i->prom == p ) {
          i->required = req;
          return;
        }
      }
      blocking_prom.push_back( blocked_promise(p,req) );
    }
    /**
     *  If all of the required promises and any optional promises then
     *  return true, else false.
     *  @todo check list
     */
    bool try_unblock( promise_base* p ) {
      if( blocking_prom.size() == 0 )  {
        return true;
     }
      bool req = false;
      for( uint32_t i = 0; i < blocking_prom.size(); ++i ) {
        if( blocking_prom[i].prom == p ) {
           blocking_prom[i].required = false;
           return true;
        }
        req = req || blocking_prom[i].required;
      }
      return !req;
    }

    void remove_blocking_promise( promise_base* p ) {
      for( auto i = blocking_prom.begin(); i != blocking_prom.end(); ++i ) {
        if( i->prom == p ) {
          blocking_prom.erase(i);
          return;
        }
      }
    }

    void timeout_blocking_promises() {
      for( auto i = blocking_prom.begin(); i != blocking_prom.end(); ++i ) {
        i->prom->set_exception( std::make_shared<timeout_exception>() );
      }
    }
    void set_exception_on_blocking_promises( const exception_ptr& e ) {
      for( auto i = blocking_prom.begin(); i != blocking_prom.end(); ++i ) {
        i->prom->set_exception( e );
      }
    }
    void clear_blocking_promises() {
      blocking_prom.clear();
    }

    bool is_complete()const { return complete; }



    bc::detail::fcontext_t       my_context;
    fc::context*                caller_context;
    stack_allocator*            stack_alloc;
    priority                     prio;
    //promise_base*              prom; 
    std::vector<blocked_promise> blocking_prom;
    time_point                   resume_time;
   // time_point                   ready_time; // time that this context was put on ready queue
    fc::context*                next_blocked;
    fc::context*                next_blocked_mutex;
    fc::context*                next;
    fc::thread*                 ctx_thread;
    bool                         canceled;
#ifndef NDEBUG
    const char*                  cancellation_reason;
#endif
    bool                         complete;
    task_base*                   cur_task;
    uint64_t                     context_posted_num; // serial number set each tiem the context is added to the ready list
  };

} // naemspace fc 

