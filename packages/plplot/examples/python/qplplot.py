import qt
from qt import QWidget 
import plplot
import plplot_widget
from Numeric import array

class qplplot(QWidget):

   def __init__(self,parent=None,name=None  ):
      qt.QWidget.__init__(self,parent,name)
      print " QWidget OK"
#      self.setMouseTracking (1 )
      self.is_created=0
      self.count=1
      print " ??????"
      self.ipls=plplot_widget.plpartialInitXw()
      print " ??????"
      print  self.ipls     
      self.isactive=0

   def __getattr__(self,attr):
      print attr
      attr="pl"+attr
      print "pl+++",attr
      self.__attr = getattr(plplot,attr)
      if(self.isactive==0):
        return self.message
      print "AAA",self.__attr ,"AAA"
      return self.__wrap
      
   def __wrap(self, *args):
      print " wrap "
      
      if(hasattr(plplot,"semaphore")):
       if(plplot.owner is not self): 
           plplot.semaphore.acquire()
           plplot.owner=self
      self.set_stream()
#      self.__attr(*args)
      if(hasattr(plplot,"semaphore")):
       plplot.semaphore.release()

   def resize(self, w,h):
      qt.QWidget.resize(self,w,h)
      print " OK "

   def attiva(self):
      self.isactive=1
      self.installEventFilter(self)
      self.show()   
  
   def message(self,*args):
        print "active this widget first "

   def mousePressEvent(self, qmev):
      self.xstart=qmev.x()
      self.ystart=qmev.y()
      
   def mouseMoveEvent (self,  qmev ) :
         print "MOVE IN PYTHON"
         if(hasattr(plplot,"semaphore")):
      	  if(plplot.owner is not self): 
            plplot.semaphore.acquire()
            plplot.owner=self
         self.set_stream()


         if(qmev.state()==1):
           x=qmev.x()
           y=qmev.y()
           (xa,ya)=plplot.plPixel2U(self.xstart,self.ystart )
           (xb,yb)=plplot.plPixel2U(x, y)
           plplot.plmoveimage(xb-xa,yb-ya)
           plplot.plNoBufferNoPixmap()
           plplot.plRestoreWrite2BufferPixmap()

            
         else:
           plplot.plNoBufferNoPixmap()

           (xu,yu)=plplot.plPixel2U(qmev.x(), qmev.y())
           (xmin,xmax,ymin,ymax)=plplot.plPgvpw()
           plplot.plline( [xmin,xmax],[yu,yu])
           plplot.plline( [xu,xu],[ymin, ymax])

         
           plplot.plRestoreWrite2BufferPixmap()


         
         if(hasattr(plplot,"semaphore")):
           plplot.semaphore.release()

        
   def set_stream(self):
      plplot.plsstrm(self.ipls)

   def resizeQUIET(self):
      if(hasattr(plplot,"semaphore")):
        if(plplot.owner is not self): 
           plplot.semaphore.acquire()
           plplot.owner=self
      plplot.plsstrm(self.ipls)
      plplot_widget.plresize(self.width(), self.height())
      if(hasattr(plplot,"semaphore")):
        plplot.semaphore.release()

   def exposeQUIET(self):
      if(hasattr(plplot,"semaphore")):
        if(plplot.owner is not self): 
           plplot.semaphore.acquire()
           plplot.owner=self
      plplot.plsstrm(self.ipls)
      plplot_widget.plexpose()
      if(hasattr(plplot,"semaphore")):
        plplot.semaphore.release()

   def eventFilter(self,ob,ev):

      if(hasattr(plplot,"semaphore")):
        if(plplot.owner is not self): 
           plplot.semaphore.acquire()
           plplot.owner=self
      plplot.plsstrm(self.ipls)

      print " intercettato event ",  ev.type() 
      
      if(self.count==1):
        self.Init()
        self.is_created=1
        self.count=2
  
      if(self.is_created):
        print " EVENTO = ",ev.type() , " ", qt.QEvent.Resize
        if(ev.type()==qt.QEvent.Paint):
	  plplot_widget.plexpose()
          timer =  qt.QTimer( self )
	  self.connect( timer, qt.SIGNAL("timeout()"), self.exposeQUIET)
	  timer.start( 0, 1)
          return 1
        elif(ev.type()==qt.QEvent.Resize):
          print " RESIZE "
          self.resizeQUIET()  
          return 1
        else:
          if(ev.type()==12) :
	   plplot_widget.plexpose();
      if(hasattr(plplot,"semaphore")):
        plplot.semaphore.release()
  
      return 0

   def Init(self):
    print " init  \n"

    if(hasattr(plplot,"semaphore")):
      if(plplot.owner is not self): 
           plplot.semaphore.acquire()
           plplot.owner=self
    plplot.plsdev("xwin")
    plplot.plsxwin(self.winId())
    plplot.plspause(0)
    plplot.plinit()
    plplot.plbop()
    if(hasattr(plplot,"semaphore")):
     plplot.semaphore.release()
