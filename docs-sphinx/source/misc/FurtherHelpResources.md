Further Help Resources
======================

##Table of Contents
1. [Dakota Reference Manual](#dakota-reference-manual)
2. [Screencasts](#screencasts)
3. [Contact Information](#contact-information)
4. [Reporting Dakota GUI Errors](#reporting-dakota-gui-errors)

##Dakota Reference Manual<a name="dakota-reference-manual"></a>

The Dakota Reference Manual is directly embedded in the GUI for easy access.  There are multiple ways to launch it:

* Right-click on a Dakota keyword in the Project Explorer view and select "Open in Dakota Reference Manual."  This will take you directly to the manual page relevant to that keyword.

![alt text](img/DakotaStudyIntro_Editing_4.png "Open in Dakota Reference Manual")

* Click on a **?** icon in the Dakota Visual Editor.  This will also take you directly to the manual page relevant to that keyword.

* Select Help > Dakota Reference Manual from the top-level menus.  This opens the Dakota Reference Manual’s introductory page.

To configure which version of the Dakota Reference Manual to launch, use the Dakota Preferences page.

The Dakota Reference Manual is also available [on the Dakota website](https://dakota.sandia.gov/content/latest-reference-manual).

##Screencasts<a name="screencasts"></a>

In addition to this manual, the Dakota team has developed [a series of video screencasts](https://www.youtube.com/playlist?list=PLouetuxaIMDo-NMFXT-hlHYhOkePLrayY) that are aimed at helping users learn how to use Dakota at varying stages of complexity.  These screencasts complement the material presented in the Dakota GUI manual, since many of the videos use the GUI to demonstrate Dakota usage.

* **Introduction to Dakota**
 * [Running a Simple Dakota Example](https://www.youtube.com/watch?v=ofi13UTq_Is)
 * [Input File Format](https://www.youtube.com/watch?v=f1l8DIXd9Gs)
 * [More Method Examples with Rosenbrock](https://www.youtube.com/watch?v=jPd5zarUs1o)
* **Sensitivity Analysis**
 * [Introduction to Sensitivity Analysis](https://www.youtube.com/watch?v=YshRCgm_f1Y)
 * [Sampling](https://www.youtube.com/watch?v=dnqoUCw6wSo)
 * Variance-Based Decomposition (coming soon)

##Contact Information<a name="contact-information"></a>

* Use the [community mailing list](mailto:dakota-users@software.sandia.gov) to get help from the Dakota community, as well as Dakota team members.
* The Dakota team also periodically monitors YouTube comments on screencast tutorials.

##Reporting Dakota GUI Errors<a name="reporting-dakota-gui-errors"></a>

If you would like to report a bug found in the Dakota GUI, please perform the following steps:

* If the Error Log view is not open in the Dakota GUI, then go to Window > Show View > Other… then select Error Log from the list of the views.
* In the Error Log view, there is a table of error history. Find the error that occurred for you, then double-click it.
* Copy the text in the Exception Stack Trace textbox.
* Use the dakota-users mailing list (see the Dakota mailing lists page) to contact a Dakota GUI developer.  Be sure to include:
 * Context about what you were doing in the GUI to cause the error.
 * The proper Exception Stack Trace copied from the Error Log view.