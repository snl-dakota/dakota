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

/*********************************************************/ 
/*	Direct Optimizer Library - Constrained Version   */ 
/*	Sequential Implementation		         */ 
/*	Steven Cox 				         */ 
/*	March 2002                                       */ 
/*	Object Oriented Version 			 */
/*********************************************************/ 
 
#include <acro_config.h>
#include <utilib/_math.h>
#include <scolib/PDirect.h>
#ifdef ACRO_HAVE_MPI
#include <mpi.h>
#endif

using namespace std;

namespace scolib {

using namespace direct;

PDIRECT::PDIRECT()
{
pmethod_str = "original";
startcond_str = "original";
dd = true;
switcher = 5;

create_parameter("multidivide", dd,
	"<bool>","true",
	"If true then a parent box can not be divided again until all of its\n\tchild boxes have been analyzed");

create_parameter("pmethod", pmethod_str,
	"<string>","original",
	"Algorithmic method used to parallelize DIRECT:\n\
\t  original - wait for all points to be finished at each iteration before\n\
\t\tgenerating the next set of points.\n\
\t  continuous - calculate the next set of potentially optimal boxes using\n\
\t\tthe boxes that have been finished once there are no more points\n\
\t\twaiting to be distributed to the analysis processors.\n\
\t  hybrid - start using original DIRECT and switch to continuous DIRECT\n\
\t\tafter 'switcher' iterations have passed");

create_parameter("startcond", startcond_str,
	"<string>","original",
	"Method used to initialize parallel DIRECT:\n\
\t  original - Divide initial design space using the pattern normally\n\
\t\tused by DIRECT without waiting for the points to be analyzed for\n\
\t\tthe continuous version of DIRECT.  The dimensions are divided in\n\
\t\trandom order and the largest boxes are divided until enough\n\
\t\tpoints are generated for all on the analysis processors to have\n\
\t\tat least 2 points to analyze before starting DIRECT\n\
\t  even - The design space is divided into equal size and shape boxes\n\
\t\tevenly distributed through the design space to provide at least\n\
\t\t2 boxes per analysis processor.");

create_parameter("switcher", switcher,
	"<int>","5",
	"Controls the switching point for hybrid parallelization");
}


void PDIRECT::reset()
{
DIRECT::reset();

if (pmethod_str == "original")
   pmethod = 1;
else if (pmethod_str == "continuous")
   pmethod = 2;
else if (pmethod_str == "hybrid")
   pmethod = 3;
else
   EXCEPTION_MNGR(runtime_error,"PDIRECT::reset - bad value \"" << pmethod_str << "\" for option \"pmethod\"");

if (startcond_str == "original")
   startcond = 1;
else if (startcond_str == "even")
   startcond = 2;
else
   EXCEPTION_MNGR(runtime_error,"PDIRECT::reset - bad value \"" << startcond_str << "\" for option \"startcond\"");
}


/*******************************************************************/ 
/* Main Program                                                    */ 
/*******************************************************************/ 
void PDIRECT::minimize() 
  { 
#if 0
#ifdef ACRO_HAVE_MPI
   const int master = 0;   
   int added = 0;
   int i, l, m, msize, complete; //, pid
   point temppt(dimen,ncon);
   vector<direct::point> tempcenter;
   double a[3];  
   double* answer = new double[2+ncon];	//results from calculation  
   double starttime, endtime, tot_time; 
   vector<double> tempvec(dimen+1);
   list<int>::iterator pfiter;
   list<direct::vec>::iterator citer, citer2;
   list<direct::simple_sort>::iterator iorder;
   list<direct::mesg>::iterator irequest;
   vector<double>  sendvec;   	// = new double[dimen+1];
   char name[50];
   char message[50];
   int namelen, igen, prepoints, thought_so;
      
   converge = 0; 
   nsort = 0;  
   start = 0;
   wait2 = 0;
 
   MPI_Status status, status2 ;  
   
   pool.empty();
   pfin.clear();
   child.clear();
   index.clear();
   xbest.resize(dimen);
   
   //MPI_Init(&argc,&argv) ;  
   MPI_Comm_size(MPI_COMM_WORLD, &nproc) ;  
   MPI_Comm_rank(MPI_COMM_WORLD, &pid) ;
	
   if (pid != master)
     {

 	MPI_Get_processor_name(name, &namelen);
	sprintf(message,"pid = %d, name = %s\n", pid, name);
	MPI_Send(message, strlen(message)+1, MPI_CHAR, master, 1, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);
     }
/* Master processor part */  
  
   if (pid == master)  
     {  
	ofstream out0("collect.dat", (ios::out | ios::app)); 
      	MPI_Get_processor_name(name, &namelen);
      	out0 << "pid = " << pid << ", name = " << name << endl;
	   for (int dest = 1; dest < nproc; dest++)
		{
		  MPI_Recv(message, 51, MPI_CHAR, dest, 1, MPI_COMM_WORLD, &status);
		  out0 << message;
		}
	MPI_Barrier(MPI_COMM_WORLD);
  
// open file "high.dat" here <----------------------------- 
	ofstream out1("high.dat", (ios::out | ios::app)); 
 
// open file "points.dat" here <------------------------ 
	ofstream out2("points.dat"); 

// open file "plot.dat" here <------------------------ 
	ofstream out3("plot.dat"); 
    
    for (int j = 1; j <= 2; j++)
 	for (i = 1; i< nproc; i++)
	  pfin.push_back(i);

	starttime = MPI_Wtime(); 
	
 	if (agressive == 3)
    	   igen = 2*dimen+1+nproc*2;		// gives nproc*2 extra points in addition to boxes from 1st itteration
  	else
   	   igen = nproc*2-1;

  if (startcond == 1)
      {
// find sizes and center point
	for (i = 0; i < dimen; i++) 
	  { 
     	    tempvec[i+1] = (xu[i]+xl[i])/2;		//center point of box 
     	    temppt.x[i] = (xu[i]+xl[i])/2;		//center point of box 
	    temppt.l[i] = 1;				//each box is 1 unit long in each dimension 
   	    size[i] = (xu[i]-xl[i]);			//length of box side 
	  } 
     	tempvec[0] = 0;					//center point of box 
	center.push_back(temppt);
	pool.push(tempvec);
	
 
	printf("start  igen = %d\n", igen);
	do  
	  { 
	    initial();  
	    Divider();  
	    if ((center.size() >= igen) || (pmethod == 1) || (pmethod == 3))
 	      break;
		 	
	} while (1);				// loop until enough points to generate 2 for each with 15 extra for queue  

cout << "end initial generating loop" << endl;

	if ((pmethod == 1) || (pmethod == 3))			// boxes divided in initial box division		 
 	   prepoints = 1;
 	else
 	   prepoints = center.size();

      }
  else if (startcond == 2)
      {   printf("starting cond 2\n");
      	for (i = 0; i < dimen; i++)  
	   size[i] = (xu[i]-xl[i]);				//length of box side  
        
        initial2(igen);
        
 	prepoints = center.size();
      }
  printf("finished start\n");    
//MPI_Request check;
//  MPI_Isend(&(xu[0]),dimen,MPI_DOUBLE,1,10, MPI_COMM_WORLD,&check);	//Non blocked send  
int temp;
cin >> temp;

  Distribute();
  
  start = 1;
cin >> temp;

//MAIN LOOP 
  do { 
     m = 0; 
 
//POLL FOR COMPLETED BOXES  

     do { 
	msize = 2+ncon; 
 
	MPI_Recv(&answer[0],msize,MPI_DOUBLE, MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status); 	// Blocks until new message received  
//cout << answer[1] << endl;
	complete = (int)answer[0];		//index2 of finished point					// Status contains the PID of the sending processor  
	center[complete].Val = answer[1];	//put function value into array 
 	for (i=0; i < ncon; i++)
 	   center[complete].c[i] = answer[i+2];
 	  
	if (a[1] < fbest)
	  {
	    fbest = answer[1];
	out1 << complete << "  " << fbest << endl;
	    ibest = complete;
	    for (int i =0; i< dimen; i++)
	       xbest[i] = center[complete].x[i];
	  }
 
/************ print test area below here ********************/ 
printf("Recv job no.%3d from proc. %d ", (int)answer[0], status.MPI_SOURCE);  
printf(" result  %13.8f \n", answer[1]);  
/************ print test area above here ********************/

	do{
	irequest = request.begin();
	  MPI_Test(&(request.front().check),&thought_so,&status2);		// Check result put in thought_so
	  if (thought_so)
	    request.pop_front();
	}while(!thought_so);
//cout << "wait2 = " << wait2 << endl;

	if (wait2 == 2)				//Only generate more points if one processor is finished
	  {
	    pfiter = pfin.begin();
	    int k = pfin.size();
	    for (int j=0; j < k; j++)
	      {
	        if (*pfiter == status.MPI_SOURCE) {
		   m = 1;
		   break;
                   }
	      }
	    pfin.push_back(status.MPI_SOURCE);
	  }
			
 
	if (wait2 == 0)				//Only send more points if not waiting for all analyses to complete
	  {
	    msize = dimen+1;  
	    int dest = status.MPI_SOURCE; 	// destination is the rank that just send message, in status  

	     mesg tempsend;
	     list<direct::mesg>::iterator irequest;

	     tempsend.send = pool.front();
	     request.push_back(tempsend);

/********* print test area below here ****************/ 
printf("Send job no.%3d send to proc no %d : \n", (int)tempsend.send[0], dest);  
//cout << pool.front();  
/********* print test area above here ****************/  
  	    
	    irequest = request.end();
	    irequest--;
	      MPI_Isend(&((*irequest).send[0]),msize,MPI_DOUBLE,dest,10, MPI_COMM_WORLD,&((*irequest).check));	//Non blocked send  
	    pool.pop();
	  
//CHECK FOR ENOUGH BOXES IN QUEUE  
	    if ((pmethod == 1) || ((pmethod == 3) && (itter < switcher)))
	      { 
	        if (pool.size() <= 0)			//generate more points NOW if none left  
	  	  wait2 = 1;			// wait2=1 means wait for all jobs to finish 
	      }
	    else
	      { 
	        if (pool.size() <= 0) 		// wait for 1 processor to finish before generating more  
	          {
	            wait2 = 2;			// wait2=2 means wait for one processor to completely finish  
		    printf("wait2 = 2   %d\n", status.MPI_SOURCE);
	          }
	      }
	
	  }

	if (complete < prepoints) {
	  insert(complete);
          added = 1;
          }
	else
	  { 
//cout << "checking if all children finished " << endl;
 
//Check to see if all of the child boxes from a parent are finished if not a prepoints box
	    citer = child.begin();
	    citer2 = child.end();
	    while (citer != citer2)
	      {

	        l=0;					// l= 0 means all boxes done
		for (int j=1; j <= (dimen*2); j ++) 
	   	  { 
	   	    if ((*citer).V[j] != 0)
	    		if(center[(*citer).V[j]].Val == 0)
	    		  {
	    		    l = 1;				// l=1 means some children not analyzed
	    	 	    break;	
	    	 	  }
	   	  }
		
		if (l == 0)
		  {
// divide boxes
		    divide2(citer);  				// send the parent index to divide on
		    
		    for (int j=1; j <= (dimen*2); j ++) 
	   	      if ((*citer).V[j] != 0)
	    		{
	    		  complete = (*citer).V[j];
//PUT COMPLETED BOX INTO LIST  
//       printf("box added is %d  from dimension %d\n", complete, ((j+1)/2));
			  insert(complete);  
		          added = 1;
			}
	       	
//  Remove parent and children from children array since they have been added to the index

	  	     child.erase(citer);
		     citer--;
		  }
		citer++;
	    citer2 = child.end();
	      }
 	  }	
        if (added) {
	   order.sort();
	   added = 0;
	   }

 
// Check to see if you should generate new points
	if (wait2 == 1)
	  {

	    if (child.size() <= 0)
	      {
		m = 1;					//m = 1 means go to graham
		wait2 = 0;
	      }
	  } 
	
     } while (m != 1);  

     iorder = order.begin();
/**************Stopcond 1***************/
     if ((min_boxsize > 0.0) && ((*iorder).size < min_boxsize))	//stop when smallest box reaches certain size 
       {
          converge = 1;
          if (wait2 == 2)
             wait2 = 1;
       }

/**************Stopcond 2***************/
     if ((max_iters > 0) && (itter >= max_iters))           	//stop when max number of iterations exceeded 
       {
          converge = 1;
          if (wait2 == 2)
             wait2 = 1;
       }

/**************Stopcond 3***************/
     if ((max_neval > 0) && (center.size() > max_neval))           		//stop when max number of function evaluations exceeded 
       {
          converge = 1;
          if (wait2 == 2)
             wait2 = 1;
       }

/*************End Stopconds*************/

     if (converge != 1)
       { 
cout << "generating more " << endl;
 
   	 itter++;
// locate potentially optimal points 
	 if (method == 1)
	    Consort();
	 else if (method == 2)
    	    Graham(); 

// Divide Potentially optimal boxes
	 Divider(); 
	 
	 Distribute();
       }
     else
       {
          cout << "waiting for final points" << endl;
          // Send stop signal to all processors
    	  MPI_Request tempreq;
	  sendvec.resize(dimen+1,0);
	  for (int i=1; i < nproc; i++)	
	    MPI_Isend(&sendvec[0],msize,MPI_DOUBLE,i,0, MPI_COMM_WORLD,&tempreq);	//Non blocked send 
       }

        
   } while ((converge != 1) || (wait2 == 1 ));  
msize = dimen+1;
cout << "completed DIRECT" << endl;

// Find best point
cout << fbest << " " << xbest << endl;
	best_val = center[0].Val;
	best_point = center[0].x;
	for (int i =center.size()-1; i >=0; i--)
	   if (center[i].Val < best_val)
	     {  
		best_val = center[i].Val;
		best_point = center[i].x;
		ibest = i;
	     }
	
	xbest = best_point;
	fbest = best_val;
	
//cout << best_val << " " << best_point << endl;
	
//	best_point = xbest;
//	best_val = fbest;

	endtime = MPI_Wtime();
	tot_time = endtime - starttime;

cout << "about to print" << endl;

// Print results to file 
	cout << center[1].Val << endl;
	out0 << tot_time << "  " << center.size()<< "  "<< itter <<"  " << ibest << "  " << best_val << endl;
	out0 << best_point << endl;
	 
	cout<<"\n"<< center.size()<< "  " <<ibest<<"  "<< best_val<<"  "<< itter; 
	for (int i=0; i<dimen; i++)
	   cout << "  " << best_point[i];
	cout << endl;

	out1<<"\n"<< center.size()<< "  " << fbest<<"  "<< itter; 
	for (int i=0; i<dimen; i++)
	   out1 << " " << xbest[i];

	for (int j = 0; j < center.size(); j++)
	 {
	   out2 << j << "  " << center[j].Val;
	    for (int i=0; i<dimen; i++)
	     out2 << " " << center[j].x[i];
	    for (int i=0; i<dimen; i++)
	     out2 <<" " << center[j].l[i];
	  out2 << endl;
	 }

// close file "collect.dat" here <-----------------------------
	out0.close(); 
// close file "high.dat" here <-----------------------------
	out1.close(); 
// close file "points.dat" here <--------------------------- 
	out2.close(); 
// close file "plot.dat" here <----------------------------- 
	out3.close(); 
     }	

/*****************************************************************************/  
/* Slave processor part */ 
 
   if (pid != master)  
     {  
	vector<double> y(dimen);		//used to pass position to eval
        double* data_in = new double[dimen+3];	//input array for slave proc. 
	vector<double> g(ncon);			//used to pass position to eval
        MPI_Request req1 ;  
        
	msize = dimen + 1;  
	int msend = ncon + 2;  
	while (1)  
	  { 
   		MPI_Recv(data_in,msize,MPI_DOUBLE,master,MPI_ANY_TAG, MPI_COMM_WORLD,&status);  
if (status.MPI_TAG == 0)
  cout << "processor " << pid << " received " << data_in[0] <<  " tag " << status.MPI_TAG << endl;
   		if (status.MPI_TAG == 0)  
  			break;  
   		else  
   		  {  
			for (i=0; i< (dimen); i++)  
				y[i] = data_in[i+1];  
 			answer[0] = data_in[0]; 
 
			problem.EvalF(y,answer[1]);  

              		if (ncon >=1)
	      	 	  {
	   		     problem.EvalCF(y,g);		//call constraints here
	   		     for (int j=0; j< ncon; j++)
	   		        answer[2+j] = g[j];
            		  }

       	     		MPI_Test(&req1,&thought_so,&status2);
   			MPI_Isend(&answer[0],msend,MPI_DOUBLE,master,status.MPI_TAG,MPI_COMM_WORLD,&req1);  
		  }  
 	  }  
     }  

   //MPI_Finalize();  
	return ;
#endif
#endif
  } 
 
/************************************************************************/ 
/* Subroutine to divide the potentially optimal boxes into smaller boxes*/ 
/************************************************************************/ 
 
void PDIRECT::Divider(vector<unsigned int>& optimal) 
{ 
   double lng, z; 
   int *divide = new int[2*dimen];
   int *split = new int[2*dimen];
   int *dim = new int[dimen]; 
   int m, tmin, net, sz; 
   list<direct::vec>::iterator citer, citer2;
   vector<double> tempvec;
cout << "start divider" << endl;
 
   sz = optimal.size();
 cout << optimal.size() << endl;
   for (m = 0; m < sz; m++) 
     { 
        unsigned int j;
        unsigned int k = optimal[m]; 

        if (dd == true)
	  	  { 
	  	  	j=0;
	  	  	citer = child.begin();
	   	 	citer2 = child.end();
	    	
	    	while (citer != citer2)
	     	  { 
	      	    if ((*citer).V[0] == k)
	              {
	                j=1;
	                break;
		  		  }
				citer++;
	     	  }
	  		if(j == 1)
	    	  continue;
		  }

        lng = 0.0;														//only divide dimensions that are longer than the rest 
        z = l2_norm(center[k].l); 
        tmin = 99999;
        for (unsigned i = 0; i < dimen; i++) 
          { 
      	     dim[i] = 0; 
             if (center[k].l[i] > lng)			//finding longest dimension 
         	lng = center[k].l[i]; 
          } 
      
     	if (z < min_boxsize) 
          continue;						 
 cout << z << endl;
             
	j=0;
        net = 0; 
        for (unsigned i = 0; i < dimen; i++) 
          if (fabs(center[k].l[i]-lng) < (0.001*min_boxsize)) //allows for round off error
            { 
              if (division == 0)
                {
                  dim[j] = i; 
                  j++; 
                  net++;			// # of dimensions being divided													//# of dimensions to divide 
                }
              else
                {
                  if (division_counter[i] < (unsigned int)tmin)			// divide longest side that was divided least
                    {
                     dim[0] = i;
                     tmin = division_counter[i];
                    }
                  net = 1;
                } 
            }
        if (division == 1)
           division_counter[dim[0]]++;					// increment number of divisions for side being divided
//cout << "generate new pts" << endl;
           
        for (int i = 1; i <= (net*2); i++)	//create the new points 
      	  center.push_back(center[k]);
      	  
        for (int i = 0; i < net; i++)   
	  { 
	    center[center.size()+2*i].x[dim[i]] += center[center.size()+2*i].l[dim[i]]/3*range[dim[i]];   
	    center[center.size()+2*i+1].x[dim[i]] -= center[center.size()+2*i+1].l[dim[i]]/3*range[dim[i]];   
	    center[k].l[dim[i]] = center[k].l[dim[i]]*1/3;   
	  }   
	tempvec.resize(dimen+1);

        for (int i = 0; i < (net*2); i++)  
	  {
	    for (unsigned int j =0; j< dimen; j++) 
              tempvec[j+1] = center[i+center.size()].x[j];
            center[i+center.size()].Val = 0;
            tempvec[0] = i+center.size();
//cout << i << "  " << tempvec[0]<< endl;

     	    pool.push(tempvec);
	  } 
 
        
// move center[k] in the global index 
 
#if 0
BUG?
	z=l2_norm(center[k].l); 
 
      	list<direct::simple_sort>::iterator giter1, giter2;
      	giter1 = order.begin();
      	giter2 = order.end();
    	
      	while(giter1 != giter2)
      	  {
    	    if ((*giter1).order == k)
    	      {
    	        (*giter1).size() = z ;
    	        break;
    	      }
    	    giter1++;
    	  }
 
        order.sort();
#endif
    	        	
//end of move 

//	 cout << start << " " << pmethod << endl;
 
      if ((start == 0) && (pmethod != 1) && (pmethod != 3))
	{ 
//randomly sort points  
	  for (int i = 0; i < (2*net); i++)	//start with the points in the order that they were generated  
	    divide[i] = i+center.size();  
		 
	  for (int i = 0; i < (2*net); i++)	//randomly switch each point with some other   
	    {     
	      int t = rand();   
	      int j = (int)((t/RAND_MAX)*2*net);  
	      int temp = divide[i]; 
	      divide[i] = divide[j];  
	      divide[j] = temp;  
	    }  
//cout <<"divide" << endl;

//divide the new boxes based on random order   
	  for (int i = 0; i < net; i++)				//divide the boxes based on the sorted values 
	    {   
	      split[i*2] = 0;                                 //has the box been divided completely yet   
	      split[i*2-1] = 0;   
	    } 
  	  for (int i = 0; i < (2*net); i++)  
	    { 
	      k = divide[i]-center.size();  
	      if (split[k] != 1)  
		{  
		  for (unsigned int j = center.size(); j < (center.size()+2*net); j++) 
		    {
		      if (split[j-center.size()] != 1) 
			center[j].l[dim[(k)/2]] = center[j].l[dim[(k)/2]]/3;    
//	printf(" box %d was just divided in dimension %d \n",j, dim[(k+1)/2]);  
				
                    }
		  split[k] = 1;  
		  if ((k/2) == ((k+1)/2))  
		    split[k+1] = 1;   
		  else   
		    split[k-1] = 1;   
		} 
	    } 
	}
      else 
	 {
	     direct::vec temp2; 
// put divided boxes into children array
		temp2.V.clear();
		temp2.V.resize(2*dimen+1);
		temp2.V[0] = k;
		int ll = center.size() ;
             for (int j =0; j< net; j++) 
                {
		  temp2.V[dim[j]*2+1] = ll;
	          ll++;
		  temp2.V[dim[j]*2+2] = ll;
		  ll++;
		}
//	cout << "pushing into child " << temp2.V << endl;	
			child.push_back(temp2);

	   } 
           
     } 
cout << "done" << endl;

   delete [] divide;
   delete [] split;
   delete [] dim; 

} 

/************************************************************************/   
// Subroutine for initial selection for divisions 		               */   
/************************************************************************/   
   
void PDIRECT::initial(vector<int>& optimal)   
{   
   int  k;   
   list<direct::simple_sort>::iterator oiter, oiter2, oiter3;
      
#if 0
// sort points by distance  
   
BUG?
	for (unsigned int i = 0; i < center.size(); i++)
	  order.push_back(direct::simple_sort(i,l2_norm(center[i].l),1));
	order.sort();
#endif
	
// end sorting   
	optimal.clear();
	oiter = order.begin();
	oiter2 = oiter;
	oiter++;
 	k = 1;  
	optimal.push_back(((*oiter).order));
	oiter3 = order.end();
	
	while (1)  
	  {  
		if (oiter == oiter3)
		   break;
		if (((*oiter).size() -0.001*min_boxsize) > (*oiter2).size())			//if the box stays the same size  
		   {  
			optimal.push_back((*oiter).order); 
			k++;  
		   }   
		oiter ++;
		oiter2++;
	 }
	cout << "finished sort " << endl;
//	nact = k;
//	order.clear();
}
  

/************************************************************************/   
// Subroutine Initial2					               */   
/************************************************************************/   

void PDIRECT::initial2(int igen)
{
  int *a = new int[dimen];
  int *b = new int[dimen];			// indexes for division
  int j,l,m, prod;
  direct::point temppt(dimen,ncon);
  vector<double> tempvec(dimen+1);
  
  for (unsigned i=0; i< dimen; i++)
    {
       a[i] = 1;
       b[i] = 1;
    }
    
// Find number of divisions to make

  m = 0;
  
  do {
    for (unsigned i = 0; i< dimen; i++)
     {
      a[i] ++;
      prod = 1;
      for (unsigned j = 0; j< dimen; j++)
         prod = prod *a[j];
      if (prod >= igen)
         {
            m = 1;
     	    break;
    	 }
     }
    
  }while (m == 0);

// Divide Boxes  
  j = 0;
  m = 0;
  do {
     tempvec[0] = j;
     temppt.Val = 0;
     
     for (unsigned i = 0; i < dimen; i++)
        {
           temppt.x[i] = x_lower[i];
           temppt.x[i] += range[i]/(a[i]*(b[i]-0.5));
           temppt.l[i] = 1./a[i];
           tempvec[i+1] = temppt.x[i];
        }
     center.push_back(temppt);
     pool.push(tempvec);
     j ++;
     
     cout << tempvec << endl;
     
     for (unsigned k =0; k< dimen; k++)
        if (b[k] < a[k])
           {  
              b[k] ++;
              for (l = 0; (unsigned int)l < k; l++)
                 b[l] =1;
              break;
           }
        else if (k == dimen-1)
           m = 1;
     } while (m == 0);
  
  //numb = j;
}


/************************************************************************/   
// Subroutine Distribute				                */   
/************************************************************************/   

int converge=1;
int itter;

void PDIRECT::Distribute()
{ 
#ifdef ACRO_HAVE_MPI
  direct::mesg tempsend;
  list<direct::mesg>::iterator irequest;
  vector<double>  sendvec(dimen+1);   	
 
  if (converge == 1)
    {
      wait2 = 0;
      return;
    }
  
  int msize = dimen+1; 
  tempsend.send.resize(dimen+1);

  if ((pmethod == 1) || ((pmethod == 3) && (itter < switcher)))
    for (int j = 1; j <= 2; j++)			// loop twice to send 2 messages to each process  
      { 
	for (int i = 1; i < nproc; i++)  
	  {  
	     if (pool.size() ==0)	// Stop if you run out of points
	       break; 
	     tempsend.send = pool.front();
	     request.push_back(tempsend);
	     
	     cout << request.back().send  << endl;
	     
/********* print test area below here ***************/
printf("Send to proc no %d, job no. %d\n", i,(int)tempsend.send[0]);  
//cout << pool.front() << endl;  
/********* print test area above here ****************/ 
//int temp;
//cin >> temp;
	irequest = request.end();
	irequest--;
	      MPI_Isend(&(request.back().send[0]),msize,MPI_DOUBLE,i,10, MPI_COMM_WORLD,&(request.back().check));	//Non blocked send  
	      pool.pop(); 
	   } 
	if ((itter == switcher) && (pmethod == 3))
	   pfin.pop_front();
	
      } 

  if ((pmethod == 2) || ((pmethod == 3) && (itter >= switcher)))
    for (int j = 1; j <= 2*nproc; j++)			// loop twice to send 2 messages to each process  
      { 
	 if (pool.size() == 0)		// Stop if you run out of points
	       break; 
	 if (pfin.size() == 0)		// Stop if you run out processors
	       break; 
				  
	     tempsend.send = pool.front();
	     request.push_back(tempsend);
/********* print test area below here ****************/
printf("Send to proc no %d, job no. %d", *pfin.begin(),(int)tempsend.send[0]);  
//	cout << " : " << pool.front();  
printf("\n");  
/********* print test area above here ****************/  
 
	irequest = request.end();
//	irequest--;
	      MPI_Isend(&((*irequest).send[0]),msize,MPI_DOUBLE,*pfin.begin(),10, MPI_COMM_WORLD,&((*irequest).check));	//Non blocked send  
	      pool.pop(); 
	      pfin.pop_front();
	
      } 

  if (pool.size() ==0)
    {
       if ((pmethod == 1) || ((pmethod == 3) && (itter < switcher)))
         wait2 = 1;
       if ((pmethod == 2) || ((pmethod == 3) && (itter >= switcher)))
         wait2 = 2;
    }
  else 
     wait2 =0;
     
  return;
#endif
} 

/************************************************************************/   
/* Subroutine Insert   --  adds new points to the list	                */   
/************************************************************************/   
void PDIRECT::insert(int notnew)   
{      
order.push_front(simple_sort(notnew, &center));
order.sort();
nsort++ ; 
}   
  
/************************************************************************/   
/* Subroutine Divide2					                */   
/************************************************************************/   
 
void PDIRECT::divide2(list<direct::vec>::iterator parent) 
{ 
   double z; 
   int* divide = new int[2*dimen];
   int* split = new int[2*dimen];
   int* dim = new int[dimen]; 
   int i, k, j, start=0, net; 
cout << "find first child" << endl;

   for (unsigned i = 1; i < 2*dimen; i+= 2)
	if ((*parent).V[i] >=1 )
 	  {
	    start = (int)(*parent).V[i];
	    break;
	  }

//divide the new boxes based on the values at the centers 

   for (unsigned i = 0; i < dimen; i++) 
     dim[i] = 0; 				// the dimensions to divide on

// find number of new boxes
   net = 0;  				// the number of new boxes
   j=0;
   for (unsigned i=1; i<=(2*dimen); i+= 2)
     {
        if ((*parent).V[i] >= 1)
	  {
	    dim[j] = i/2;
	       net++;
	       j++;
	  }
     }

   if (constraint_method == 1)
     {
cout << "calc pens for ConDIRECT" << start << endl;

//        lng = 0
        z = center[start].l[0];														//only divide dimensions that are longer than the rest 
/*        for (i = 0; i < dimen; i++) 
          { 
             if (center[start].l[i] > lng)			//finding longest dimension 
         	    lng = center[start].l[i]; 
          } 
*/
	for (i=start+1; i< start+2*net;i++)
 	  {
//            z=lng;			// z = distance between points 
 	    //s0 += fabs((center[i].Val-center[start].Val)/z);
 	    for (j=0; j< ncon; j++)
 	       constraint_scale[j] += fabs((center[i].response.constraint_values()[j]-center[start].response.constraint_values()[j])/z);
 	  } 
     }
	  
//sort points 
   for (i = 0; i < (2*net); i++) 
     divide[i] = 0; 
 
   for (i = start; i < (start+2*net); i++) 
     for (j = 0; j < (2*net); j++) 
       { 
       	 if (divide[j] == 0) 
           { 
             divide[j] = i; 
             break; 
           } 
         if (center[i].Val - center[divide[j]].Val < -0.00000000000001) 
           { 
             for (k = (2*net-1); k >= (j+1); k--) 
               divide[k] = divide[k-1]; 
             divide[j] = i; 
             break; 
           } 
      	}
 
   for (i = 0; i < net; i++)									//divide the boxes based on the sorted values 
     { 
       split[i*2] = 0;            //has the box been divided completely yet 
       split[i*2+1] = 0; 
     } 
   for (i = 0; i < (2*net); i++) 
     { 
       k = divide[i]-start; 
       if (split[k] != 1) 
         { 
           for (j = start; j < (start+2*net); j++) 
             if (split[j-start] != 1) 
               center[j].l[dim[(k)/2]] = center[j].l[dim[(k)/2]]/3;         

	   split[k] = 1; 
           if ((k/2) == ((k+1)/2))		 
             split[k+1] = 1; 
           else 
             split[k-1] = 1; 
      	 } 
     } 
 
   delete [] divide;
   delete [] split;
   delete [] dim;
    
}

};
