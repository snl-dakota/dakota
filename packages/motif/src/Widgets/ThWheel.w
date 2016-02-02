# XfwfThumbWheel widget
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version: 1.2

@class XfwfThumbWheel (Core) @file=ThWheel

@ This is a very basic implementation of a valuator (scrolling) widget
that uses a thumbwheel as a metaphor. The thumbwheel is placed
vertically by default. The edge is turned towards the viewer and
sticking out. It can be turned up or down (or left-right) with the
mouse or with the up/down arrow keys.

With the mouse, there are two modes of operation: continuous and
discrete. Continuous movement is the result of pressing a mouse button
and dragging the mouse up or down. This is best for large, sweeping
movements. The default arrangement causes the left mouse button to
make the largest movements, the middle button makes the wheel turn
about half as fast and the third mouse button turns the wheel at a
quarter speed.

Discrete movements are good for finer control. By clicking the mouse
just below or above the wheel, the wheel turns a fixed distance (the
step size) in that direction. Here again the middle and right buttons
make the wheel turn half or a quarter of the step.

When the mouse is kept pressed for longer than a certain interval, the
wheel is repeatedly stepped, until the mouse is released.

With the keyboard, only discrete steps are possible. The default
translations assign the normal step size to the up and down keys, and
the smaller steps to the same keys combined with with Shift and
Control.

The picture of a thumb wheel is not integral to the widget, although a
default picture is provided. The picture (or rather series of
pictures) is an Image resource. To archieve a realistic `turning'
effect, at least three slightly different pictures must be provided.

@PUBLIC

@ The widget is formed by the images that represents the wheel. The
wheel includes areas where the mouse can be clicked for small,
discrete movements. The size of the image thus determines the size of
the widget. There can be any number of images, but for realisticly
looking movement at least three slightly different pictures are
needed. |numberOfPictures| gives the number of imagess, |pictures|
must be an array of at least that many images.

A default set of pictures is provided when the |pictures| array is
left undefined.

Note for application programmers: the array is not copied to private
memory, so the array must stay valid for as long as the widget exists
and changes to elements of the array have unpredictable results.

In the current implementation, these resources cannot be set with
|XtSetValues|.

	@var int numberOfPictures = 0
	@var <ImageList> XImage **pictures = NULL

@ The wheel can be either horizontal or vertical. The default is
vertical. Note that the setting of |vertical| should correspond to the
set of |pictures|.

	@var Boolean vertical = True

@ A small area along the top and bottom (or left and right) of the
widget is reserved for clicking the mouse, in order to get discrete
movements. |clickArea| is the number of pixels to reserve for this
area.

	@var Dimension clickArea = 7

@ The wheel can be turned through a range of values (integers),
indicated by |minValue| and |maxValue|. By default they are 0 and 100.
|value| is the current value, which must lie between the two
(inclusive).  Note that |minValue| may be greater than |maxValue|, but
in that case |step| (see below) should be negative.

Note for programmers: the information that is passed to callback
function always contains a value between 0 and 1, regardless of
|minValue| and |maxValue|.

	@var int minValue = 0
	@var int maxValue = 100
	@var int value = 0

@ When the user clicks on the areas below or above the wheel itself,
or when the user drags the wheel with the mouse, the wheel is turned
in fixed increments (or fractions thereof, see the translations). The
default step size is 1.

	@var int step = 1

@ When the user presses and holds the mouse for longer than
|initialDelay| milliseconds in the areas above or below the wheel, the
wheel is turned repeatedly every |repeatDelay| milliseconds.

	@var int initialDelay = 500
	@var int repeatDelay = 50

@ The |sensitivity| is the number of pixels the mouse has to move
before the wheel turns. A small value such as 3, 2 (the default) or 1
is usually best.

	@var int sensitivity = 2

@ When the wheel is turned, the ThumbWheel widget calls the callback
functions on the |scrollCallback| list. It does so with every change,
but the information in the |call_data| argument is different depending
on the event that caused the change. The |call_data| argument is a
pointer to an |XfwfScrollInfo| structure. For a full explanation of
this structure, refer to `SWIP', the Scrolling Widgets Interface
Policy. The field that are of interest here are |reason|, |flags| and
|vpos|.

The field |reason| can be either |XfwfSDrag|, |XfwfSMove|, |XfwfSUp|,
|XfwfSDown|, |XfwfSLeft|, or |XfwfSRight|. Callbacks with |reason =
XfwfSDrag| are repeatedly called when the user drags the wheel up or
down. An application may ignore them (to save time), because at the
end of the drag there will always be a callback with |reason =
XfwfSMove|. The reasons |XfwfSUp| and |XfwfSDown| (|XfwfSLeft|,
|XfwfSRight|) are used when the user clicks on the areas above or
below the wheel (repeatedly, if he holds the button down.)

In all cases, the |flags| field has the value |XFWF_VPOS| for a
vertical thumbwheel or |XFWF_HPOS| for a horizontal thumbwheel.

The |vpos| (or |hpos|) field is a real number between 0 and 1. It
indicates what the ThumbWheel widget suggests as the new position
after the event. This value is computed by adding or subtracting the
|step| from the current |value| and dividing by the total range.

When the callback is called, the |value| resource has already been
updated, so applications may use the |value| resource directly instead
of the |vpos| or |hpos| field.

	@var <Callback> XtCallbackList scrollCallback = NULL

@ In the SWIP there is a provision for scrollee widgets (i.e., the
widgets that are controlled by the wheel widget) to call back to the
wheel with their reaction to the scroll request as given by the
|scrollCallback| above. The |scrollResponse| resource (which is
read-only!) holds a pointer to a method function inside the wheel
widget. Calling this function with an appropriate |XfwfScrollInfo|
structure has the effect of updating the wheel to the values provided
by the scrollee. For more information, see the SWIP.

	@var XtCallbackProc scrollResponse = scroll_response

@EXPORTS

@ The |scroll.h| file contains the definition of |XfwfScrollInfo| and
some other things. It is exported so that applications do not have to
import it.

	@incl "scroll.h"

@TRANSLATIONS

@ Presses of each of the three mouse buttons are translated to calls
to the |turn| action. The argument indicates how fast to turn as a
percentage of the step size. An omitted argument is equivalent to 100
percent.

	@trans <Btn1Down>: turn()
	@trans <Btn2Down>: turn("50")
	@trans <Btn3Down>: turn("25")
	@trans <BtnUp>: stop_turning()
	@trans None<Key>Up: up()
	@trans !Shift<Key>Up: up("50")
	@trans !Ctrl<Key>Up: up("25")
	@trans None<Key>Down: down()
	@trans !Shift<Key>Down: down("50")
	@trans !Ctrl<Key>Down: down("25")

@PRIVATE

@ The IDs of the pictures after they have been converted to Pixmaps.

	@var Pixmap* pix

@ The currenttly displayed picture (0 to |NumberOfPictures|-1).

	@var int curpic

@ A Graphics Context is needed to draw the images.

	@var GC gc

@ A timer

	@var XtIntervalId timer

@ The amount to turn when a timed event occurs

	@var int delta

@ The reason to give when a timed event occurs.

	@var XfwfSReason reason

@ Whether a timer is currently running

	@var Boolean timer_on

@METHODS

@ When the |pictures| resource has been left |NULL|, a default set of
four images is used. They are loaded in |realize|, because only then
is there a window with the correct depth.

The images are converted to Pixmaps in the |realize| method, where the
first of them is installed as the background of the widget. The size
of the widget is set to the size of the first image (or the default
size, in case the default images are used.)

@proc initialize
{
    XGCValues values;

    if (! $pictures) $numberOfPictures = 4;	/* Built-in pictures */
    $gc = XtGetGC($, 0, &values);
    if ($pictures) {
	$width = $pictures[0]->width;
	$height = $pictures[0]->height;
    } else if ($vertical) {
	$width = 15;
	$height = 64;
    } else {
	$width = 64;
	$height = 15;
    }
    $timer_on = False;
}

@proc realize
{
    int i, stat;

    #realize($, mask, attributes);
    $pix = (Pixmap*) XtMalloc($numberOfPictures * sizeof($pix[0]));
    if ($pictures) {
	for (i = 0; i < $numberOfPictures; i++) {
	    $pix[i] = XCreatePixmap
		(XtDisplay($), XtWindow($), $pictures[i]->width,
		 $pictures[i]->height, $depth);
	    XPutImage(XtDisplay($), $pix[i], $gc, $pictures[i], 0, 0, 0, 0,
		      $pictures[i]->width, $pictures[i]->height);
	}
    } else if ($vertical) {			/* Use built-in pictures */
#ifndef NO_XPM
	stat = XpmCreatePixmapFromData
	    (XtDisplay($), XtWindow($), wheel0, &$pix[0], NULL, NULL);
	stat = XpmCreatePixmapFromData
	    (XtDisplay($), XtWindow($), wheel1, &$pix[1], NULL, NULL);
	stat = XpmCreatePixmapFromData
	    (XtDisplay($), XtWindow($), wheel2, &$pix[2], NULL, NULL);
	stat = XpmCreatePixmapFromData
	    (XtDisplay($), XtWindow($), wheel3, &$pix[3], NULL, NULL);
#else
	$pix[0] = XCreatePixmapFromBitmapData
	    (XtDisplay($), XtWindow($), wheel0_bits, wheel0_width,
	     wheel0_height, BlackPixelOfScreen(XtScreen($)),
	     WhitePixelOfScreen(XtScreen($)), $depth);
	$pix[1] = XCreatePixmapFromBitmapData
	    (XtDisplay($), XtWindow($), wheel1_bits, wheel1_width,
	     wheel1_height, BlackPixelOfScreen(XtScreen($)),
	     WhitePixelOfScreen(XtScreen($)), $depth);
	$pix[2] = XCreatePixmapFromBitmapData
	    (XtDisplay($), XtWindow($), wheel2_bits, wheel2_width,
	     wheel2_height, BlackPixelOfScreen(XtScreen($)),
	     WhitePixelOfScreen(XtScreen($)), $depth);
	$pix[3] = XCreatePixmapFromBitmapData
	    (XtDisplay($), XtWindow($), wheel3_bits, wheel3_width,
	     wheel3_height, BlackPixelOfScreen(XtScreen($)),
	     WhitePixelOfScreen(XtScreen($)), $depth);
#endif
    } else {					/* Horizontal */
#ifndef NO_XPM
	stat = XpmCreatePixmapFromData
	    (XtDisplay($), XtWindow($), wheel0h, &$pix[0], NULL, NULL);
	stat = XpmCreatePixmapFromData
	    (XtDisplay($), XtWindow($), wheel1h, &$pix[1], NULL, NULL);
	stat = XpmCreatePixmapFromData
	    (XtDisplay($), XtWindow($), wheel2h, &$pix[2], NULL, NULL);
	stat = XpmCreatePixmapFromData
	    (XtDisplay($), XtWindow($), wheel3h, &$pix[3], NULL, NULL);
#else
	$pix[0] = XCreatePixmapFromBitmapData
	    (XtDisplay($), XtWindow($), wheel0h_bits, wheel0h_width,
	     wheel0h_height, BlackPixelOfScreen(XtScreen($)),
	     WhitePixelOfScreen(XtScreen($)), $depth);
	$pix[1] = XCreatePixmapFromBitmapData
	    (XtDisplay($), XtWindow($), wheel1h_bits, wheel1h_width,
	     wheel1h_height, BlackPixelOfScreen(XtScreen($)),
	     WhitePixelOfScreen(XtScreen($)), $depth);
	$pix[2] = XCreatePixmapFromBitmapData
	    (XtDisplay($), XtWindow($), wheel2h_bits, wheel2h_width,
	     wheel2h_height, BlackPixelOfScreen(XtScreen($)),
	     WhitePixelOfScreen(XtScreen($)), $depth);
	$pix[3] = XCreatePixmapFromBitmapData
	    (XtDisplay($), XtWindow($), wheel3h_bits, wheel3h_width,
	     wheel3h_height, BlackPixelOfScreen(XtScreen($)),
	     WhitePixelOfScreen(XtScreen($)), $depth);
#endif
    }

    XtVaSetValues($, XtNbackgroundPixmap, $pix[0], NULL);
    $curpic = 0;
}

@ There is no |set_values| method yet.

@ There is no need for an |expose| method, since the X server will
take care of drawing the background pixmap of the widget.

@ The |scroll_response| method can be called by scrollees that want to
synchronize the thumbwheel to themselves. It sets the thumbwheel's
|value| according to the |vpos| field. When the |reason| was a request
and not a notification, the thumbwheel in turn calls its
|scrollCallback| to notify scrollees that the thumbwheel has changed.

@proc scroll_response(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget $ = (Widget) client_data;
    XfwfScrollInfo *info = (XfwfScrollInfo *) call_data;
    XfwfScrollInfo new_info;

    if ($vertical) {
	if (info->flags & XFWF_VPOS)
	    $value = ($maxValue - $minValue) * info->vpos + $minValue + 0.5;
    } else {
	if (info->flags & XFWF_HPOS)
	    $value = ($maxValue - $minValue) * info->hpos + $minValue + 0.5;
    }
    if (info->reason != XfwfSNotify) {
	new_info = *info;
	new_info.reason = XfwfSNotify;
	XtCallCallbackList($, $scrollCallback, &new_info);
    }
}

@ACTIONS

@ The |up| action has an optional argument |percentage|. It subtracts
|step*percentage| from the |value| resource and changes the displayed
picture. The change in picture is not (yet) related to the change in
|value|.

@proc up
{
    int percentage, d, newval, a, b;
    XfwfScrollInfo info;


    if (*num_params == 0 || sscanf(params[0], "%d", &percentage) != 1)
	percentage = 100;
    newval = $value - ($step * percentage + 99)/100; /* Round up */
    a = min($minValue, $maxValue);
    b = max($minValue, $maxValue);
    newval = min(b, max(a, newval));

    if (newval == $value) return;		/* No change */

    $value = newval;

    $curpic = ($curpic + $numberOfPictures - 1) % $numberOfPictures;
    XtVaSetValues($, XtNbackgroundPixmap, $pix[$curpic], NULL);

    if ($vertical) {
	info.reason = XfwfSUp;
	info.flags = XFWF_VPOS;
	info.vpos = 1.0 * ($value - $minValue)/($maxValue - $minValue);
    } else {
	info.reason = XfwfSLeft;
	info.flags = XFWF_HPOS;
	info.hpos = 1.0 * ($value - $minValue)/($maxValue - $minValue);
    }
    XtCallCallbackList($, $scrollCallback, &info);
}

@ The |down| action adds |step*percentage| to the |value| resource.

@proc down
{
    int percentage, d, newval, a, b;
    XfwfScrollInfo info;

    if (*num_params == 0 || sscanf(params[0], "%d", &percentage) != 1)
	percentage = 100;

    newval = $value + ($step * percentage + 99)/100; /* Round up */
    a = min($minValue, $maxValue);
    b = max($minValue, $maxValue);
    newval = min(b, max(a, newval));

    if (newval == $value) return;		/* No change */

    $value = newval;

    $curpic = ($curpic + 1) % $numberOfPictures;
    XtVaSetValues($, XtNbackgroundPixmap, $pix[$curpic], NULL);

    if ($vertical) {
	info.reason = XfwfSDown;
	info.flags = XFWF_VPOS;
	info.vpos = 1.0 * ($value - $minValue)/($maxValue - $minValue);
    } else {	
	info.reason = XfwfSRight;
	info.flags = XFWF_HPOS;
	info.hpos = 1.0 * ($value - $minValue)/($maxValue - $minValue);
    }
    XtCallCallbackList($, $scrollCallback, &info);
}

@ The |turn| action looks at the position of the mouse and if it is in
the top or bottom (left or right, if horizontal) area of the widget it
turns the wheel in discrete, timed steps, as long as the button
remains pressed. If the mouse is elsewhere, the position is remembered
and while the mouse remains pressed, the wheel is turned an amount
proportional to the distance that the mouse moves from its initial
position.

The optional argument gives the percentage of the |step| that the
wheel is turned with every event.

@def BMask = (Button1Mask|Button2Mask|Button3Mask|Button4Mask|Button5Mask)

@proc turn
{
    int percentage, d, mouse, rx, ry, m, x, y, m1, oldvalue, newval, a, b;
    Window root, child;
    unsigned int mask;
    XfwfScrollInfo info;

    if (*num_params == 0 || sscanf(params[0], "%d", &percentage) != 1)
	percentage = 100;
    d = ($step * percentage + 99)/100;

    /* Get starting mouse position */
    mouse = $vertical ? event->xbutton.y : event->xbutton.x;

    if (mouse >= $clickArea
	&& mouse < ($vertical?$height:$width) - $clickArea) {
	/* Middle area */
	m1 = mouse;				/* Previous mouse pos. */
	oldvalue = $value;
	while (XQueryPointer(XtDisplay($), XtWindow($), &root, &child,
			     &rx, &ry, &x, &y, &mask)
	       && (mask & BMask)) {
	    m = $vertical ? y : x;
	    if (m != m1) {
		m1 = m;

		newval = oldvalue + (m - mouse)/$sensitivity * d;
		a = min($minValue, $maxValue);
		b = max($minValue, $maxValue);
		newval = min(b, max(a, newval));
		if (newval == $value) continue;	/* No change */

		$value = newval;

		$curpic = ((m - mouse)/$sensitivity % $numberOfPictures
			   + $numberOfPictures) % $numberOfPictures;
		XtVaSetValues($, XtNbackgroundPixmap, $pix[$curpic], NULL);

		info.reason = XfwfSDrag;
		if ($vertical) {
		    info.flags = XFWF_VPOS;
		    info.vpos = 1.0
			* ($value - $minValue)/($maxValue - $minValue);
		} else {
		    info.flags = XFWF_HPOS;
		    info.hpos = 1.0
			* ($value - $minValue)/($maxValue - $minValue);
		}
		XtCallCallbackList($, $scrollCallback, &info);
	    }
	}
	info.reason = XfwfSMove;
	if ($vertical) {
	    info.flags = XFWF_VPOS;
	    info.vpos = 1.0 * ($value - $minValue)/($maxValue - $minValue);
	} else {
	    info.flags = XFWF_HPOS;
	    info.hpos = 1.0 * ($value - $minValue)/($maxValue - $minValue);
	}
	XtCallCallbackList($, $scrollCallback, &info);
    } else {
	/* Upper/left or lower/right click area */
	$delta = mouse < $clickArea ? -d : d;
	if ($vertical)
	    $reason = mouse < $clickArea ? XfwfSUp : XfwfSDown;
	else
	    $reason = mouse < $clickArea ? XfwfSLeft : XfwfSRight;
	newval = $value + $delta;
	a = min($minValue, $maxValue);
	b = max($minValue, $maxValue);
	newval = min(b, max(a, newval));
	if (newval == $value) return;		/* No change */

	$curpic += mouse < $clickArea ? $numberOfPictures - 1 : 1;
	$curpic = $curpic % $numberOfPictures;
	XtVaSetValues($, XtNbackgroundPixmap, $pix[$curpic], NULL);

	$value = newval;

	info.reason = $reason;
	if ($vertical) {
	    info.flags = XFWF_VPOS;
	    info.vpos = 1.0 * ($value - $minValue)/($maxValue - $minValue);
	} else {
	    info.flags = XFWF_HPOS;
	    info.hpos = 1.0 * ($value - $minValue)/($maxValue - $minValue);
	}
	XtCallCallbackList($, $scrollCallback, &info);

	$timer_on = True;
	$timer = XtAppAddTimeOut(XtWidgetToApplicationContext($),
				 $initialDelay, timer_callback, $);
    }
}

@ The |stop_turning| action removes the timer.

@proc stop_turning
{
    if ($timer_on) {
	$timer_on = False;
	XtRemoveTimeOut($timer);
    }
}

@UTILITIES

@def abs(a) = ((a) < 0 ? -(a) : (a))
@def min(a,b) = ((a) < (b) ? (a) : (b))
@def max(a,b) = ((a) > (b) ? (a) : (b))

@ The |timer_callback| is called when a certain interval passes
without the user releasing the mouse button. The wheel is turned
another step and another time-out event is started.

@proc timer_callback(XtPointer call_data, XtIntervalId *id)
{
    Widget $ = (Widget) call_data;
    XfwfScrollInfo info;
    int newval, a, b;

    newval = $value + $delta;
    a = min($minValue, $maxValue);
    b = max($minValue, $maxValue);
    newval = min(b, max(a, newval));
    if (newval == $value) {			/* No further changes */
	$timer_on = False;
	return;
    }
    $value = newval;

    $curpic += ($reason == XfwfSUp || $reason == XfwfSLeft) ?
	$numberOfPictures - 1 : 1;
    $curpic = $curpic % $numberOfPictures;
    XtVaSetValues($, XtNbackgroundPixmap, $pix[$curpic], NULL);

    info.reason = $reason;
    info.flags = $vertical ? XFWF_VPOS : XFWF_HPOS;
    if ($vertical)
	info.vpos = 1.0 * ($value - $minValue)/($maxValue - $minValue);
    else
	info.hpos = 1.0 * ($value - $minValue)/($maxValue - $minValue);
    XtCallCallbackList($, $scrollCallback, &info);

    $timer = XtAppAddTimeOut(XtWidgetToApplicationContext($),
			     $repeatDelay, timer_callback, $);
}

@IMPORTS

@ Wheels.h contains four different images of a thumbwheel, in pixmap
(color) or bitmap (black and white) format, both either horizontal and
vertical, depending on the preprocessor symbol |NO_XPM|.

@incl "wheels.h"
@incl <stdio.h>
