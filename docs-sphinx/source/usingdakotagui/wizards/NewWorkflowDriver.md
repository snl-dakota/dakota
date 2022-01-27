New Workflow-Based Driver
=======

The Workflow-Based Dakota Driver wizard creates a [Dakota analysis driver](ExternalSimulationModelOverview.html) that is an [IWF workflow file](NextGenWorkflow.html).  Workflow files provide you with a great degree of flexibility over how you pass input variables and output responses back and forth between Dakota and an external simulation model, all without the need to write any driver scripts.

Before beginning this wizard, you will need a file that declares your expected input variables and output responses.  This could be a [BMF](BMF.html) file, or it could be a Dakota study, if you already have an appropriate Dakota study on hand.

To launch this wizard, go to File > New > Other.  Then choose Dakota > Workflow-Based Dakota Driver.

On the first page of this wizard, give your workflow file a name and location.

On the second page of this wizard, choose a variable/response source file that will auto-generate this workflow’s inputs and outputs.

![alt text](img/NewDakotaStudy_Drivers_Workflow_1.png "Get the variables and responses from our BMF file")

Now click Finish.  That was easy!

If you need help building out the driver workflow you just created, [refer to this tutorial](Wizards.html#nested-workflow-tutorial).
