/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

/**
 * \file MSApplication.h
 *
 * Defines the colin::MSApplication and colin::ResponseStatus classes.
 *
 * TODO: this needs to be documented.
 */

//
// MSApplication.h
//
// Possible modes:
//  1.	Send k messages at a time, without waiting for a response from
//	the slaves.
//  2.  Send k messages at a time, waiting for the slave to respond to the
//	last messages. (unimplemented)
//  3.  Eval on master and slaves. (unimplemented)
//
// TODO - add eval-on-master capabilities
//	Run through size-1 batch sends, and then do a batch-eval on
//	the master.  This assumes that

#ifndef colin_MSApplication_h
#define colin_MSApplication_h

#include <acro_config.h>
#include <utilib/std_headers.h>
#ifdef ACRO_HAVE_MPI
#include <mpi.h>
#include <utilib/PackBuf.h>
#endif
//#include <colin/DirectOptApplication.h>
#include <colin/Problem.h>
#include <colin/AnalysisCode.h>
#include <utilib/BasicArray.h>
#include <utilib/sort.h>
#include <utilib/LinkedList.h>  // %%%% added 6/04 plsx

#if defined (USING_PROBE)
void  probe();
#endif

namespace colin
{

template <class DomainT, class ProblemT>
class Problem;

template <class DomainT, class ProblemT>
class MSApplication;


/**
 * A utility class used by colin::MSApplication to describe the status of
 * communication between the master and slave processes.
 */
template <class DomainT, class ProblemT>
class ResponseStatus
{

#if !defined(DOXYGEN)
   friend class MSApplication<DomainT, ProblemT> ;
#endif

public:

   /// Constructor.
   ResponseStatus()
   { response = 0; }

   /// A buffer for receiving data.
   std::string recvbuf;

   /// A pointer to a response object.
   AppResponse<ResponseT>* response;

#ifdef ACRO_HAVE_MPI
   /// A pack buffer.
   PackBuffer pack;

   /// A send request object.
   MPI_Request send_request;

   /// A receive request object.
   MPI_Request recv_request;
#endif
};



#if !defined(DOXYGEN)
/**
 * The problem provides an interface for functions that are called via
 * an MPI call.
 */
template <class DomainT, class ProblemT>
class MSApplication : public DirectOptApplication<DomainT, ProblemT>
{
public:

   /// Generic constructor.
   MSApplication(Problem<DomainT, ResponseT>& prob_, int batch_size_, bool eval_on_master_);

   /// Destructor.
   virtual ~MSApplication();

   ///
   void synchronize();

   ///
   void do_agent_prioritization();

   ///
   void map_tasks_to_agents();

   ///
   int next_eval();

   ///
   int next_eval_batch();

   ///
   void order_tasks();

   ///
   void assign_task(unsigned int index);

   ///
   unsigned int num_evaluation_servers()
   {
      int size = 1;
#if defined(ACRO_HAVE_MPI)
      MPI_Comm_size(MPI_COMM_WORLD, &size);   // Compute number of processors
#endif
      return static_cast<unsigned int>(size - 1);
   }

   ///
   bool terminate_eval(const int id);

   ///
   unsigned int num_queued_evaluations()
   { return ndx; }

   ///
   void pack_agent(int q);

   ///
   void exec_server(Problem<DomainT, ResponseT> & prob);

   ///
   void terminate();

   ///
   int debug;

   ///
   Problem<DomainT, ResponseT> problem;

protected:

   ///
   bool eval_on_master;

   ///
   bool usingbatchversion;

   ///
   int batch_size;

   ///
   int batch_ctr;

   ///
   int max_pri;


   // NOTE:  The convention is that the lowest priority number is
   //   the "highest" ---i.e., most important--- priority.
   class task
   {
   public:
      DomainT pt;     // pointer to the point itself
      int * pri;        // pointer to the priority of the task
      int pt_id;      // pointer to the id that came in with the point
      ResponseT * resp;  // response obj
      int agentrank;    // which agent is in charge of this task (-1 == none)
      bool evaluated;   // true if the point has been evaluated
   };

   class agent
   {
   public:
      int rank;        // rank, in MPI terms
      int pri;         // priority of this agent
      bool idle;       // if true, has no task
      bool sent;      // if true, has been sent to a processor.
      // if true, agent has asked for work already. Give it
      // a job as soon as one comes available.
      bool request_pending;
      typename utilib::LinkedList<task>::iterator t;        // points into the tasklist
      ResponseStatus<DomainT, ResponseT> RS_data;   //  These take the place of the
      ResponseStatus<DomainT, ResponseT> RS_bufsize; //     array and bufsize vectors
   };

   ///
   BasicArray< typename utilib::LinkedList<task>::iterator > task_order;

   ///
   utilib::LinkedList<task> tasklist;

   ///
   typename utilib::LinkedList<task>::iterator t_curr;

#if defined(COUGAR) || defined(TFLOPS_SERVICE)
   ///
   std::vector< ResponseStatus<DomainT, ResponseT> * > array;

   ///
   std::vector< ResponseStatus<DomainT, ResponseT> * > bufsize;
#else
   ///
   BasicArray< ResponseStatus<DomainT, ResponseT> * > array;

   ///
   BasicArray< ResponseStatus<DomainT, ResponseT> * > bufsize;
#endif

   ///
   BasicArray< agent> agentlist;

   ///
   std::vector<int> offset;

   /// For the priorities--by rank
   std::vector <int> agent_pri;

   /// The total number of queued evaluations
   unsigned int ndx;

   ///
   std::list<int> id_flags;

   /// The next queued evaluation that needs to be sent out
   unsigned int next_pt;

   // if true, we are gathering points for later eval/prioritization
   bool gathering;

   ///
   int max_buf_size;

   ///
   int num_servers;

   ///
   int idle_servers;

   ///
   int unassigned_tasks;

   ///
   int kill_pri;

#ifdef ACRO_HAVE_MPI
   /// The MPI request object used to wait for requests from slaves
   MPI_Request feval_request;
#endif

   ///
   void start_async_batch()
   {
      ucout << "\n^^^^ Starting async batch gathering mode ^^^^" << std::endl << utilib::Flush;
      usingbatchversion = true;
      gathering = true;
   }

   ///
   void end_async_batch()
   {
      ucout << "\n^^^^ Ending async batch gathering mode ^^^^" << std::endl << utilib::Flush;
      usingbatchversion = true;
      gathering = false;
      DoEval_batch();
   }

   ///
   int num_tasks_unsent();

   /// Routines that service the requests from the slaves
   void service_requests(bool finish = false);

   ///
   void service_requests_batch(bool finish = false);

   ///
   void DoEval(const DomainT& point, int& priority, ResponseT* response,
               bool synch_flag);

   ///
   void DoEval_batch();

   ///
   void send_work_to_agent(int which);
};



#define FEVAL_REQUEST   1
#define FEVAL_RESPONSE  2
#define TERMINATE 	3
#define BUF_SIZE_MSG 	4
#define FEVAL_PT	5
#define NUM_BUFS	10


#ifdef ACRO_HAVE_MPI
/*
static void errfn()
{ EXCEPTION_MNGR(std::runtime_error,"MPI Error function!"); }

static MPI_Errhandler errhandle;
*/
#endif


//============================================================================
//
//
template <class DomainT, class ResponseT>
MSApplication<DomainT, ResponseT>
::MSApplication(Problem<DomainT, ResponseT>& prob_, int batch_size_, bool eval_on_master_)
      : debug(0),
      eval_on_master(eval_on_master_),
      usingbatchversion(false),
      batch_size(batch_size_),
      gathering(false),
      unassigned_tasks(0)
{
   problem &= prob_;
   ndx = 0;
   //app_mode = problem.app_mode();
   batch_ctr = 0;
   max_buf_size = 256;
   next_pt = 0;
   max_pri = 0;
   kill_pri = 5; // arbitrary---we'll have to set this some way that
   // is less hand-wavy.
   ucout << "++++++ Inside MSApp constructor ++++++" << std::endl << utilib::Flush;
   num_servers = num_evaluation_servers();
   idle_servers = num_servers;
   ucout << "\nIn MSAPP, num_servers == " << num_servers << std::endl;
   agentlist.resize(num_servers);
   for (int k = 0; k < (int)agentlist.size(); k++)
   {
      agentlist[k].rank = (k + 1);
      agentlist[k].pri = -1;
      agentlist[k].idle = true;
      agentlist[k].sent = false;
      agentlist[k].request_pending = false;
      agentlist[k].t = NULL;
   }
   //
   // batch sizing doesn't work right now, so set it to one.
   //
   batch_size = 1;
#ifdef ACRO_HAVE_MPI
   MPI_Irecv(0, 0, MPI_CHAR, MPI_ANY_SOURCE, FEVAL_REQUEST, MPI_COMM_WORLD,
             &feval_request);
   /*
     MPI_Errhandler_create(errfn, &errhandle);
     MPI_Errhandler_set(MPI_COMM_WORLD,errhandle);
   */
#endif
}


//============================================================================
//
//
template <class DomainT, class ResponseT>
MSApplication<DomainT, ResponseT>
::~MSApplication()
{
   for (unsigned int i = 0; i < array.size(); i++)
   {
      delete array[i];
   }

   for (unsigned int j = 0; j < bufsize.size(); j++)
   {
      delete bufsize[j];
   }

   tasklist.clear();
}


template <class DomainT, class ResponseT>
int MSApplication<DomainT, ResponseT>
::num_tasks_unsent()
{
   int n = 0;
   t_curr = tasklist.begin();
   while (t_curr != tasklist.end())
   {
      if ((t_curr->evaluated == false)
            && (t_curr->agentrank > 0)
            && (agentlist[t_curr->agentrank - 1].sent == false))
      {
         n++;
      }
      t_curr++;
   }
   return n;
}

//============================================================================
//
//
// %%%%% FIXME:: change this so it just q's the thing up into a
//    list of some sort.  Then call some other method---
//    process_tasklist or something--- to do the sending off
//    to slaves thing.
//
//
template <class DomainT, class ResponseT>
void MSApplication<DomainT, ResponseT>
::DoEval(const DomainT& point, int& priority,
         ResponseT * response, bool synch_flag)
{
#if defined (USING_PROBE)
   probe();
#endif

   //
   // Verify that the appropriate mode is used
   //
   verify(response->info->mode);

   //
   // Check to see if the point has been evaluated already.
   //
   if (response_exists(point, *response))
   {
      if (!synch_flag)
         CachedAllocator<ResponseT> :: deallocate(response);
      return;
   }

   //
   // Synchronous computation? Note: do not need to 'maintain' the
   // response id value, since this is only set in the call to Eval.
   //
   if (synch_flag)
   {
      problem.Eval(point, response->response_vector(), *response);
      //
      // Update application counters.  Note: We assume that constraints and
      // function evaluations are both being computed here.
      //
      this->nprob_ctr++;
      this->neval_ctr++;
      if ((this->num_eq_constr + this->num_ineq_constr) > 0)
         this->nconstr_ctr++;

      return;
   }

#ifdef ACRO_HAVE_MPI
   //================================================
   // %%%% Added for batch stuff
   //      ---pls, 6/04
   //================================================

   if (usingbatchversion && gathering)
   {
      response->info->id_generate = false;
      tasklist.push_back();
      t_curr = tasklist.end();
      t_curr--;
      ucout << "\nMSApp: response == " << response << std::endl;
      // cout<< "\nMSApp: response->reesponse == " << response <<std::endl;
      ucout << "\nMSApp: deref of response == " << *(response) << std::endl;
      t_curr->resp = response;
      ucout << "\nMSApp: t_curr->response == " << t_curr->resp << std::endl;
      t_curr->pt = point;
      t_curr->pri = &priority;
      if (*(t_curr->pri) < 0) *(t_curr->pri) = 2;
      ucout << "+++++ Priority is " <<  *(t_curr->pri) << std::endl;
      t_curr->pt_id = response->info->id ;
      ucout << "\nin doEval, task is == " << t_curr->pt_id << std::endl << utilib::Flush;
      assert(t_curr->pt_id != -1);
      t_curr->resp->info->id_generate = false;
      verify(response->info->mode);
      t_curr->agentrank = -1;
      t_curr->evaluated = false;
      unassigned_tasks++;
   } // if batch and gathering

   else if (usingbatchversion && !gathering)
   {
      DoEval_batch();
   } // else if batch and we have finished gathering

   else
   {
      //================================================
      ucout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  ACK  !!!!!!!!!!!!!!" << std::endl << utilib::Flush;
      //
      // Update application counters.  Note: We assume that constraints and
      // function evaluations are both being computed here.
      //

      this->nprob_ctr++;
      this->neval_ctr++;
      if ((this->num_eq_constr + this->num_ineq_constr) > 0)
         this->nconstr_ctr++;

      //
      // Pack the parameters in the array
      //
      // ..Resize the array if needed.
      //
      if (offset.size() == ndx)
      {
         array.resize(ndx + 10);
         bufsize.resize(ndx + 10);
         offset.resize(ndx + 10);
         //   task_pri.resize(ndx+10);  // %%%% added 6/4/04 ---pls
         id_flags.resize(ndx + 10);
         for (unsigned int i = ndx; i < offset.size(); i++)
         {
            array[i] = new ResponseStatus<DomainT, ResponseT>;
            bufsize[i] = new ResponseStatus<DomainT, ResponseT>;
            offset[i] = i;
            // *(task_pri[i]) = -1; // %%%% added 6/4/04 ---pls
         }
      }
      if (debug)
         ucout << "OFFSET: " << offset << std::endl << utilib::Flush;
      int id = offset[ndx];
      //
      // ..Do the packing
      //

      // use the current id for this evaluation
      response->info->id_generate = false;
      array[id]->response = response;
      array[id]->pack.reset();
      array[id]->pack << -987654321;
      array[id]->pack << *response;
      // This should work for fevals and gradients
      array[id]->recvbuf.resize(array[id]->pack.size()*2*(response->info->mode));
      array[id]->pack << point;
      if (static_cast<int>(array[id]->pack.size()) > max_buf_size)
         max_buf_size = array[id]->pack.size();
      if (debug)
      {
         ucout << "Master packing the point" << std::endl;
         ucout << "Size " << array[id]->pack.size() << std::endl;
         ucout << point;
         ucout << std::endl << *response;
         ucout << "RecvBuf-Len=" << array[id]->recvbuf.size() << std::endl;
         ucout << utilib::Flush;
      }
      response->info->id_generate = true;
      //
      // ..Show this as an available packed buffer if the batch counter is
      // ..large enough
      //
      /*
        batch_ctr++;
        if (batch_ctr == batch_size) {
        ndx++;
        batch_ctr=0;
        }
      */
      ndx++;

      //
      // Handle requests from the slaves
      //
      service_requests();
   } // else use the one-at-a-time async eval
#endif
}


//============================================================================
//============================================================================
//
//  %%%% Batch version added 6/04 pls
//
//============================================================================
//============================================================================

template <class DomainT, class ResponseT>
void MSApplication<DomainT, ResponseT>
::DoEval_batch()
{
#ifdef ACRO_HAVE_MPI


   ucout << "\n++++++++++++   In DoEval_batch   +++++++++++++" << std::endl << utilib::Flush;
   // we need to clean up the task list---get rid of tasks that have
   // been evaluated, incl. deallocating their request fields.

   max_pri = 0;
   t_curr = tasklist.begin();
   while (t_curr != tasklist.end())
   {
      if ((t_curr->evaluated == true) && (t_curr->agentrank == -1))
      {
         t_curr->resp = 0;
         t_curr = tasklist.erase(t_curr);
      }
      else if ((*(t_curr->pri) >= kill_pri) && (t_curr->agentrank == -1))
      {
         *(t_curr->pri) = (-1 * kill_pri);
         t_curr = tasklist.erase(t_curr);
      }
      else if ((t_curr->pt_id == -1) && (t_curr->agentrank == -1))
      {
         t_curr->resp = 0;
         t_curr = tasklist.erase(t_curr);
      }
      else
      {
         if (*(t_curr->pri) > max_pri)
         {
            max_pri = *(t_curr->pri);
         }
         t_curr++;
      }
   }

   task_order.resize(tasklist.size());
   t_curr = tasklist.begin();
   unsigned int k;
   for (k = 0;
         t_curr != tasklist.end()
         && k < task_order.size();
         k++)
   {
      task_order[k] = t_curr;
      assert(*(t_curr->pri) != (-1 * kill_pri));
      task_order[k] = t_curr;
      t_curr++;
   }
   assert(k == task_order.size());
   assert(task_order.size() == tasklist.size());
   order_tasks();

   // Now we prioritize the machines and do the mapping of tasks to
   //    processors.  Then we do the packing. Then we call
   //    service_requests_batch().

   do_agent_prioritization();
   map_tasks_to_agents();

   // %%%%% ACK!!! Wait a minute---we need to repack whenever
   // next_eval is called, don't we?  Or should we be packing in
   // service_requests?

   //  ucout<< "\n MSApp: before packing loop ";
   // ucout<< " \nagentlist.size ==  "<< agentlist.size()<<std::endl << utilib::Flush;
   // ..Do the packing
   for (int q = 0; q < (int)agentlist.size(); q++)
   {
      if ((agentlist[q].idle == false)
            && (agentlist[q].sent == false))
      {
         //  pack_agent(q);
         if (debug)
         {
            /*
            ucout << "Master packed this point" << std::endl;
            ucout << "Size " << agentlist[q].RS_data.pack.size() << std::endl;
            ucout << (agentlist[q].t->pt);
            ucout << std::endl << *(agentlist[q].t->resp);
            ucout << "RecvBuf-Len="<< agentlist[q].RS_data.recvbuf.size() <<std::endl;
                   ucout << utilib::Flush;
            */
         }
      }
   }
   //
   // Handle requests from the slaves
   //
   service_requests_batch();
#endif
}


//============================================================================
//
//
template <class DomainT, class ResponseT>
void MSApplication<DomainT, ResponseT>
::service_requests(bool finish)
{
#ifdef ACRO_HAVE_MPI
   if (usingbatchversion)
   {
      service_requests_batch();
      return;
   }
   bool flag = true;
   while (flag || (finish && (ndx > 0)))
   {
      ucout << "\n====== HERE1 ======= ndx=" << ndx << " next_pt=" << next_pt << "\n";
      flag = false;
      //
      // Test for requests for work from the slaves
      //
      if (next_pt < ndx)
      {
         ucerr << "\n====== HERE2 =======\n";
         int test_val = 0;
         MPI_Status status;
         MPI_Test(&feval_request, &test_val, &status);
         ucerr << "\n====== HERE3 =======" << test_val << "\n";
         if (test_val)
         {
            if (debug)
            {
               ucout << "OFFSET: " << offset << " next="
               << next_pt << " ndx=" << ndx << std::endl << utilib::Flush;
            }
            //
            // Receive the feval response
            //
            MPI_Irecv((void*)(array[offset[next_pt]]->recvbuf.data()),
                      array[offset[next_pt]]->recvbuf.size(),
                      MPI_PACKED,
                      status.MPI_SOURCE,
                      FEVAL_RESPONSE,
                      MPI_COMM_WORLD,
                      &(array[offset[next_pt]]->recv_request));
            //
            // Send off the feval request
            //
            int dest = status.MPI_SOURCE;

            MPI_Isend((void*)&max_buf_size, 1, MPI_INT, dest,
                      BUF_SIZE_MSG,
                      MPI_COMM_WORLD,
                      &(bufsize[offset[next_pt]]->send_request));
            if (debug)
            {
               ucout << "Master sending point to " << dest <<
               " bufsize=" << array[offset[next_pt]]->pack.size() << std::endl << utilib::Flush;
            }
            MPI_Isend((void*)(array[offset[next_pt]]->pack.buf()),
                      array[offset[next_pt]]->pack.size(),
                      MPI_PACKED,
                      dest,
                      FEVAL_PT,
                      MPI_COMM_WORLD,
                      &(array[offset[next_pt]]->send_request));

            next_pt++;
            MPI_Irecv(0, 0, MPI_CHAR, MPI_ANY_SOURCE, FEVAL_REQUEST,
                      MPI_COMM_WORLD, &feval_request);
         }
      }
      //
      // Check for responses from the slaves
      //
      MPI_Status status;
      for (unsigned int i = 0; i < next_pt; i++)
      {
         int id = offset[i];
         int test_val = 0;
         MPI_Test(&(array[id]->recv_request), &test_val, &status);
         if (test_val)
         {
            int count = 0;
            MPI_Get_count(&status, MPI_PACKED, &count);
            UnPackBuffer unpack(&(array[id]->recvbuf[0]), count);
            unpack >> *(array[id]->response);
            (array[id]->response)->init();
            if (debug)
            {
               ucout << "Master received response from "
               << status.MPI_SOURCE << std::endl;
               ucout << *(array[id]->response) << utilib::Flush;
            }
            if (finish == false)
               id_flags.push_back(array[id]->response->info->id);
            CachedAllocator<ResponseT> :: deallocate(array[id]->response); // What does this do to the pointers ??????
            std::swap(offset[i], offset[--next_pt]);
            std::swap(offset[next_pt], offset[--ndx]);
         }
      }
   }
#endif
}
//========================================================
//      %%%% added 6/04 pls
// Note that finish  is true only if we are
// calling this from the synchronize() method. We ignore
// it anyway, as this method is used only for asynchronous
// communications with priorities added.
//========================================================
template <class DomainT, class ResponseT>
void MSApplication<DomainT, ResponseT>
::service_requests_batch(bool finish) // default is false,
{
#ifdef ACRO_HAVE_MPI
   // ucout<<"\n++++++++++++  Inside Service Requests Batch +++++++++++++"
   // <<std::endl << utilib::Flush;
   bool flag = true;

   // send tasks to any agents who have already asked and can now
   // accept work.
   for (unsigned int j = 0; j < agentlist.size(); j++)
   {
      if ((agentlist[j].request_pending == true)
            && !(agentlist[j].idle))
      {
         send_work_to_agent(j + 1);
         agentlist[j].request_pending == false;
         agentlist[j].sent == true;
      }
   }

   int finished = num_tasks_unsent();
   while (flag && (finished > 0))
   {
      //
      // Test for requests for work from the slaves
      //
      int test_val = 0;
      MPI_Status status;

      MPI_Test(&feval_request, &test_val, &status);
      flag = (test_val > 0);
      int dest = status.MPI_SOURCE;
      if (status.MPI_SOURCE == MPI_ANY_SOURCE)
      {
         // Print error message?  This should not happen.
      }
      if (test_val > 0 && status.MPI_SOURCE > 0)
      {
         if (status.MPI_ERROR != 0)
         {
            ucout << "\n******Value of MPI_error == "
            << status.MPI_ERROR << std::endl << utilib::Flush;
         }
         // int dest = status.MPI_SOURCE;

         if (!(agentlist[dest-1].idle) && !(agentlist[dest-1].sent))
         {
            send_work_to_agent(dest);
            finished--;
         } // if agent has a task assigned

         else
         {
            // We post the receive here to clear the queue and
            //  set agentlist[dest-1].request_pending to true so
            //  we don't post the receive again later.
            MPI_Irecv(0, 0, MPI_CHAR, MPI_ANY_SOURCE, FEVAL_REQUEST,
                      MPI_COMM_WORLD, &feval_request);
            agentlist[dest-1].request_pending = true;
            flag = false;
         }
      } // if test_val
      else
      {
         flag = false;
      }
   } // end while

   //
   // Check for responses from the slaves
   //

   MPI_Status status;
   for (unsigned int i = 0; i < agentlist.size(); i++)
   {
      if ((agentlist[i].idle == false)
            && (agentlist[i].sent == true)
            && (agentlist[i].t != NULL)
            && (agentlist[i].t->evaluated == false))
      {
         int test_val = 0;
         MPI_Test(&(agentlist[i].RS_data.recv_request), &test_val, &status);
         if (test_val)
         {
            int count = 0;
            MPI_Get_count(&status, MPI_PACKED, &count);
            UnPackBuffer unpack(&(agentlist[i].RS_data.recvbuf[0]), count);
            unpack >> *(agentlist[i].RS_data.response);
            (agentlist[i].RS_data.response)->init();
            if (debug)
            {
               ucout << "****Master received response from " << status.MPI_SOURCE << std::endl;
               ucout << *(agentlist[i].RS_data.response);
               ucout << std::endl << "Value is ";
               ucout << agentlist[i].RS_data.response->function_value() << std::endl;
               ucout << utilib::Flush;
            }
            agentlist[i].t->evaluated = true;
            agentlist[i].request_pending == false;
         }
      }
   }
#endif
}


template <class DomainT, class ResponseT>
void MSApplication<DomainT, ResponseT>
::send_work_to_agent(int dest)
{
#ifdef ACRO_HAVE_MPI
   // MPI_Status status;
   MPI_Irecv((void*)(agentlist[dest-1].RS_data.recvbuf.data()),
             agentlist[dest-1].RS_data.recvbuf.size(),
             MPI_PACKED,
             dest,
             FEVAL_RESPONSE,
             MPI_COMM_WORLD,
             &agentlist[dest-1].RS_data.recv_request);
   //
   // If this procesor is due for a task, send off the
   //  feval request
   //
   // %%%% moved this from DoEval (batch version) 6/04 ---pls
   this->nprob_ctr++;
   this->neval_ctr++;
   if ((this->num_eq_constr + this->num_ineq_constr) > 0)
   {
      this->nconstr_ctr++;
   }

   MPI_Isend((void*)&max_buf_size, 1, MPI_INT, dest,
             BUF_SIZE_MSG,
             MPI_COMM_WORLD,
             &agentlist[dest-1].RS_bufsize.send_request);
   if (debug)
   {
      /*
      ucout<< "\nMaster sending  point of task id # "
      << agentlist[dest-1].t->pt_id << std::endl;
      ucout << "Master sending point to " << dest << 
        " bufsize=" << agentlist[dest-1].RS_data.pack.size() << std::endl;
      ucout << utilib::Flush;
      ucout<< "Sending 2nd msg to agent "<< dest<<std::endl;
      ucout<<"packbuf = ";
      ucout<<(int)agentlist[dest-1].RS_data.pack.buf() << std::endl;
      ucout<<(int)*(agentlist[dest-1].RS_data.pack.buf()) << std::endl;
      ucout<<"response obj  = "; 
      ucout<< *(agentlist[dest - 1].RS_data.response) << std::endl<<utilib::Flush;
      */
   }
   MPI_Isend((void*)(agentlist[dest-1].RS_data.pack.buf()),
             agentlist[dest-1].RS_data.pack.size(),
             MPI_PACKED,
             dest,
             FEVAL_PT,
             MPI_COMM_WORLD,
             &agentlist[dest-1].RS_data.send_request);

   agentlist[dest-1].sent = true;
   agentlist[dest-1].idle = false;

   if (agentlist[dest-1].request_pending == false)
   {
      MPI_Irecv(0, 0, MPI_CHAR, MPI_ANY_SOURCE, FEVAL_REQUEST,
                MPI_COMM_WORLD, &feval_request);
   }
   else
   {
      agentlist[dest-1].request_pending = false;
   }
#endif
} // send_work_to_agent

//============================================================================

template <class DomainT, class ResponseT>
void MSApplication<DomainT, ResponseT>
::synchronize()
{
//
// Finish off the last batch
//
   /*
   if (batch_ctr > 0) {
      ndx++;
      batch_ctr=0;
      }
   */
//
// Service all requests until ndx==0
//
   service_requests(true);
}

//============================================================================
//  %%%% Added two methods, 6/04 ---pls
//============================================================================

// For now, just put them in the order we get them.
template <class DomainT, class ResponseT>
void  MSApplication<DomainT, ResponseT>
::do_agent_prioritization()
{
   agent_pri.resize(num_servers);
   for (int i = 0; i < num_servers; i++)
   {
      agent_pri[i] = (i + 1);
      agentlist[i].pri = (i + 1);
   }

}

// For now, all we do is give tasks to processors
// in order.
//==================================================================
// We are going to have to keep a sorted list of some
// sort---probably an array of pointers.
template <class DomainT, class ResponseT>
void  MSApplication<DomainT, ResponseT>
::map_tasks_to_agents()
{
   int len = agentlist.size();
   bool easyway = false;

   //  This just does them in list order.
   if (easyway == true)
   {
      t_curr = tasklist.begin();
      for (int i = 0;
            ((i < len) && (t_curr != tasklist.end()));
            i++)
      {
         while ((t_curr != tasklist.end())
                && ((t_curr->agentrank > -1) || (t_curr->pt_id == -1)))
         {
            t_curr++;
         }
         if ((agentlist[i].idle)
               && (t_curr != tasklist.end())
               && (t_curr->agentrank == -1))
         {
            agentlist[i].t = t_curr;
            agentlist[i].idle = false;
            agentlist[i].sent = false;
            agentlist[i].RS_data.response = agentlist[i].t->resp;
            idle_servers--;
            unassigned_tasks--;
            t_curr->agentrank = agentlist[i].rank;
            if (debug)
            {
               ucout << "\nJust mapped task ID # "
               << agentlist[i].t->pt_id
               << " to agent number "
               << agentlist[i].rank << std::endl << utilib::Flush;
            }
            pack_agent(i);  //added %%%%% pls
            t_curr++;
         }
      }
   }
   // Else we take priority into account.
   else
   {
      len = agentlist.size();
      int ag, to;
      to = 0;
      // cout<< "\nTask order size is "<<task_order.size();
      // cout<< "  and agentlist size is "<< agentlist.size();
      // cout<<std::endl;
      for (ag = 0;
            ((ag < len) && (to < (int)task_order.size()));
            ag++)
      {

         //  cout<<" "<<ag<<" ";
         while ((to < (int)task_order.size())
                && ((task_order[to]->agentrank > -1)
                    || (task_order[to]->pt_id == -1)))
         {
            //	cout<<"  Agentrank is "<<task_order[to]->agentrank;
            //	cout<<" and pt_id is "<<task_order[to]->pt_id<<std::endl;
            to++;
         }

         if (to >= (int)task_order.size())
         {
            //	cout<<" *bye* ";
            break;
         }
         if ((agentlist[ag].idle)
               && (task_order[to]->agentrank == -1))
         {
            agentlist[ag].t = task_order[to];
            agentlist[ag].idle = false;
            agentlist[ag].sent = false;
            agentlist[ag].RS_data.response = agentlist[ag].t->resp;
            idle_servers--;
            unassigned_tasks--;
            task_order[to]->agentrank = agentlist[ag].rank;
            if (debug)
            {
               ucout << "\nJust mapped task ID # "
               << agentlist[ag].t->pt_id
               << " to agent number "
               << agentlist[ag].rank << std::endl << utilib::Flush;
            }
            pack_agent(ag);  //added %%%%% pls
            to++;
         }
      } // for
   }
   //  cout<<" Ok, let's look at the task_order array and the ";
   // cout<<"actual tasklist. "<<std::endl;
   /*
   t_curr = tasklist.begin();
   int blahblah = 0;
   while(t_curr != tasklist.end()){
     cout<< "Id = " <<t_curr->pt_id <<" = "<< task_order[blahblah]->pt_id;
     cout<< " and agent rank is "<<t_curr->agentrank<<" = ";
     cout<< task_order[blahblah]->agentrank<<std::endl;
     t_curr++;
     blahblah++;
   }
   */

}
// %%%% added 7/04 pls

template <class DomainT, class ResponseT>
void MSApplication<DomainT, ResponseT>
::order_tasks()
{
   // ***Kludge alert***
   //             ---->  Multiply priority by next power of 10
   //   larger than task_order.size().  Add rank in task_order
   //   array.  Then sort them.  Sort will be stable.  When sorted,
   //   undo the mult/add; while doing so, order the task array
   //   accordingly.
   // It is extremely unlikely that we will have anywhere close to
   //   int_max/10  tasks, and it is also unlikely that we will
   //   have  vast numbers of different priorities, so I am not
   //   worried about overflowing.
   //  ----%%%%% pls, 7/04

   /*
   cout<<"Before we sort, we will look at the task_order array and the ";
   cout<<"actual tasklist. "<<std::endl;
   cout<<"max_pri = "<<max_pri<<std::endl;
   t_curr = tasklist.begin();
   int blahblah = 0;
   while(t_curr != tasklist.end()){
     cout<< "Id = " <<t_curr->pt_id <<" = "<< task_order[blahblah]->pt_id;
     cout<< ", agent rank is "<<t_curr->agentrank<<" = ";
     cout<< task_order[blahblah]->agentrank;
     cout<< ", and priority is "<<  *(task_order[blahblah]->pri)<<std::endl;
     t_curr++;
     blahblah++;
   }
   */
   unsigned int i, num;
   // cout<< "\nmax_pri == "<<max_pri<<std::endl;
   if (max_pri == 0)return;
   assert(task_order.size() == tasklist.size());
   BasicArray< typename utilib::LinkedList<task>::iterator > temp_order;
   temp_order.resize(task_order.size());
   // The LinkedList class does appear to have an overloaded assignment
   // operator for its iterators, so I'll use it here.
   for (unsigned int qw = 0; qw < (task_order.size()); qw++)
   {
      temp_order[qw] = task_order[qw];
   }
   long multiplier = 10;
   bool finished = false;
   int starter = max_pri > (int)task_order.size() ? max_pri + 1 : task_order.size() + 1;
   while (!finished)
   {
      starter = (int)(starter / 10);
      if (starter > 0)
      {
         multiplier *= 10;
      }
      else finished = true;
   }
   //int length = (int)task_order.size();
   BasicArray<int> helper;
   helper.resize(task_order.size());
   int tempint;
   //  Do the reshuffling in this loop.  See comment above.
   for (int q = 0; q < (int)task_order.size(); q++)
   {
      tempint = (int)(*((task_order[q])->pri));
      if ((tempint < 0) ||
            (task_order[q]->pt_id == -1))
      {
         tempint = max_pri;
      }
      helper[q] = q + (tempint * multiplier);
   }
   //  cout<<"task_order size == "<<task_order.size();
   // cout<<", templong == "<<templong;
   // cout<<", and multiplier ==  "<<multiplier<<std::endl;
   sort(helper);
   num = 0;
   for (i = 0; i < task_order.size(); i++)
   {
      num = (unsigned int)((helper[i]) % multiplier);
      //  cout<<"\nnum == "<<num;
      task_order[i] = temp_order[num];
   }
   /*
   cout<< std::endl;
   cout<<"Now that the sort is done, the task_order array and the ";
   cout<<"tasklist look like this: "<<std::endl;
   t_curr = tasklist.begin();
   blahblah = 0;
   while(t_curr != tasklist.end()){
     cout<< "Id = " <<t_curr->pt_id <<" = "<< task_order[blahblah]->pt_id;
     cout<< ", agent rank is "<<t_curr->agentrank<<" = ";
     cout<< task_order[blahblah]->agentrank;
     cout<< ", and priority is "<<  *(task_order[blahblah]->pri)<<std::endl;
     t_curr++;
     blahblah++;
   }
   */
}

// %%%% added 7/04 pls
// The parameter "index" is the index into the agentlist
//    array.
//
// PRE:  task_order array has been sorted, with best (lowest #)
//       priority first, worst last.
template <class DomainT, class ResponseT>
void MSApplication<DomainT, ResponseT>
::assign_task(unsigned int index)
{}

//============================================================================
//
//
template <class DomainT, class ResponseT>
int MSApplication<DomainT, ResponseT>
::next_eval()
{
   if (usingbatchversion)
      return next_eval_batch();

   service_requests();

   while (id_flags.size() > 0)
   {
      int id = id_flags.front();
      id_flags.pop_front();
      if (this->dead_ids.size() > 0)
      {
         //
         // Eliminate id's that are dead
         //
         std::set<int>::iterator findval = this->dead_ids.find(id);
         if (findval != this->dead_ids.end())
            this->dead_ids.erase(findval);
         else
            return id;
      }
      else
         return id;
   }
   return -1;
}

//  Go through the agentlist. Return the id of the task
//  of the first agent whose task is done.  Then
//  mark the agent as idle and detach the task.
template <class DomainT, class ResponseT>
int MSApplication<DomainT, ResponseT>
::next_eval_batch()
{
   int id = -1;
#if 0
   WEH - figure out how to use the dead_ids in this case...

   // cerr<< "\n======In next_eval_batch=======\n";
   service_requests_batch();
   do_agent_prioritization(); // reprioritize here
   map_tasks_to_agents();

   unsigned int i;
   for (i = 0; i < agentlist.size(); i++)
   {
      if ((agentlist[i].idle == false)
            && (agentlist[i].t->evaluated == true)
            && (agentlist[i].t->agentrank > 0))
      {
         id = agentlist[i].t->pt_id;
         if (debug)
         {
            /*
            ucout<<"\n======In next_eval_batch,";
            ucout<<"*****ID == "<< agentlist[i].t->pt_id <<". " ;
            ucout<<" Resp val in agent is "
               <<agentlist[i].RS_data.response->function_value() 
               <<" and in task is "<<agentlist[i].t->resp->function_value()
               <<std::endl; 
            ucout<< "Rank is "<<agentlist[i].rank << std::endl << utilib::Flush;
            */
         }
         agentlist[i].idle = true;
         agentlist[i].sent = false;
         agentlist[i].t->agentrank = -1;
         agentlist[i].t->pt_id = -1;
         //*(agentlist[i].t->pri) = max_pri;
         agentlist[i].t = NULL;
         //agentlist[i].RS_data.response = 0;
         do_agent_prioritization(); // reprioritize here
         map_tasks_to_agents();
         service_requests_batch();
         break;
      }
   }
#endif
   return id;
}

//============================================================================
//
//
template <class DomainT, class ResponseT>
bool MSApplication<DomainT, ResponseT>
::terminate_eval(const int id)
{
   return false;
}



template <class DomainT, class ResponseT>
void MSApplication<DomainT, ResponseT>
::pack_agent(int q)
{
   agentlist[q].RS_data.response =  agentlist[q].t->resp;
   // use the current id for this evaluation
   agentlist[q].RS_data.response->info->id_generate = false;
   agentlist[q].RS_data.pack.reset();
   agentlist[q].RS_data.pack << -987654321;
   agentlist[q].RS_data.pack << *(agentlist[q].t->resp);
   // This should work for fevals and gradients
   agentlist[q].RS_data.recvbuf.resize(agentlist[q].RS_data.pack.size()*
                                       2*(agentlist[q].t->resp->info->mode));
   agentlist[q].RS_data.pack << (agentlist[q].t->pt);
   if (static_cast<int>(agentlist[q].RS_data.pack.size()) > max_buf_size)
      max_buf_size = agentlist[q].RS_data.pack.size();
   ucout << "\nMaster packing  point for task id # "
   << agentlist[q].t->pt_id << " == "
   << agentlist[q].RS_data.response->info->id << std::endl << utilib::Flush;
   assert(agentlist[q].t->pt_id ==  agentlist[q].RS_data.response->info->id);
}
//============================================================================
//
//
template <class DomainT, class ResponseT>
void MSApplication<DomainT, ResponseT>
::exec_server(Problem<DomainT, ResponseT> & prob)
{
#if defined (USING_PROBE)
   probe();
#endif

#ifdef ACRO_HAVE_MPI
   int rank;
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);   // Compute processor id

   ResponseT response;
   MPI_Status status;
   PackBuffer pack;
   MPI_Request buf_request;
   MPI_Request pt_request;
   MPI_Request termination_request;
   MPI_Request send_request;
   MPI_Request ready_request;
   bool termination = false;

   std::string recvbuf;
   bool received_bufsize = false;
   bool request_flag = false;

   int buffersize;
   MPI_Irecv((void*)&buffersize, 1, MPI_INT, 0, BUF_SIZE_MSG,
             MPI_COMM_WORLD, &buf_request);
   MPI_Irecv(0, 0, MPI_CHAR, 0, TERMINATE,
             MPI_COMM_WORLD, &termination_request);
   MPI_Isend(0, 0, MPI_CHAR, 0, FEVAL_REQUEST, MPI_COMM_WORLD, &ready_request);

   while (!termination)
   {
      //
      // Get the buffer size msg
      //
      if (!received_bufsize)
      {
         int test_val = 0;
         MPI_Test(&buf_request, &test_val, &status);
         if (test_val)
         {
            if (((unsigned int)buffersize) > recvbuf.size())
               recvbuf.resize(buffersize);
            received_bufsize = true;
            if (debug)
            {
               ucout << "Processor " << rank
               << " received buffer size msg" << std::endl << utilib::Flush;
            }
            MPI_Irecv((void*)(&buffersize), 1, MPI_INT, 0, BUF_SIZE_MSG,
                      MPI_COMM_WORLD, &buf_request);
            MPI_Irecv((void*)(recvbuf.data()),
                      recvbuf.size(), MPI_PACKED, 0, FEVAL_PT,
                      MPI_COMM_WORLD, &pt_request);
         }
      }
      //
      // .. then get the optimization point
      //
      if (received_bufsize)
      {
         int test_val = 0;
         MPI_Test(&pt_request, &test_val, &status);
         if (test_val)
         {
            //
            // Unpack the point
            //
            int count = 0;
            MPI_Get_count(&status, MPI_PACKED, &count);
            UnPackBuffer unpack(&(recvbuf[0]), count);
            int tmp;
            unpack >> tmp;
            if (tmp != -987654321)
            {
               EXCEPTION_MNGR(std::runtime_error, "Corrupted message from the master: check=" << tmp);
            }
            unpack >> response;
            unpack >> prob.get_point();
            //if (debug)
            if (1)
            {
               ucout << "Processor: " << rank
               << " Received parameters (test=OK)" << std::endl;
               ucout << "Count " << count << " bufsize "
               << recvbuf.size() << std::endl;
               ucout << prob.get_point();
               ucout << std::endl << response << std::endl;
               ucout << utilib::Flush;
            }
            //
            // Evaluate the point.  Keep the response.id in a temporary int,
            // since the call to Eval will rewrite it and we need to return
            // the _global_ response id, not the local value that is assigned
            // in the Eval call.
            //
            int tmp_id = response.info->id;
            ucout << "In slave, id is " << response.info->id << utilib::Flush;
            if (debug)
            {
               ucout << "Processor: " << rank << " starting evaluation." << std::endl;
               ucout << tmp_id << std::endl;
               ucout << response << std::endl;
               ucout << utilib::Flush;
            }
            ucout << "\nSlave calling eval now...";
            prob.Eval(response.request_vector(), response, response.info->mode);
            response.info->id = tmp_id;
            ucout << "\nNow, in slave, id is " << response.info->id << utilib::Flush;
            if (debug)
            {
               ucout << "Processor: " << rank << " finished evaluation." << std::endl;
               ucout << response << std::endl << "Value is ";
               ucout << response.function_value() << std::endl;
               ucout << utilib::Flush;
            }
            //
            // Send the result
            //
            pack.reset();
            pack << response;
            if (request_flag)
            {   // Just to be sure that the prior Isend finished
               MPI_Wait(&send_request, &status);
            }
            MPI_Isend((void*)pack.buf(), pack.size(), MPI_PACKED, 0,
                      FEVAL_RESPONSE, MPI_COMM_WORLD, &send_request);
            if (1)
            {
               //   if (debug)
               ucout << "Processor: " << rank << " sent result:  "
               << FEVAL_RESPONSE << std::endl;
               ucout << utilib::Flush;
            }
            MPI_Isend(0, 0, MPI_CHAR, 0, FEVAL_REQUEST, MPI_COMM_WORLD, &ready_request);
            if (1)
            {
               //  if (debug)
               ucout << "Processor " << rank << " waiting for a message." << std::endl << utilib::Flush;
            }
            request_flag = true;

            received_bufsize = false;
         }
      }

      //
      // Check for termination message
      //
      int test_val = 0;
      MPI_Test(&termination_request, &test_val, &status);
      if (test_val)
         termination = true;
   } // while not terminated

#endif
} // end exec_server()


//============================================================================
//
//
template <class DomainT, class ResponseT>
void MSApplication<DomainT, ResponseT> :: terminate()
{
//
// This synchronous send may be less efficient than an asynchronous
// send, but it's more convenient for now.
//
#ifdef ACRO_HAVE_MPI
   int size;
   MPI_Comm_size(MPI_COMM_WORLD, &size);   // Compute # of processors

   for (int i = 1; i < size; i++)
      MPI_Send(0, 0, MPI_CHAR, i, TERMINATE, MPI_COMM_WORLD);
#endif
}

};
#endif


#endif
