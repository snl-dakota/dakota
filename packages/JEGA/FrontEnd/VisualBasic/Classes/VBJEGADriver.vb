' =============================================================================
#Region "File Preamble"
' =============================================================================
'     PROJECT:
'
'         John Eddy's Genetic Algorithms (JEGA) Visual Basic Front End
'
'     CONTENTS:
'
'         Declaration of Class VBJEGADriver.
'
'     NOTES:
'
'
'
'     PROGRAMMERS:
'
'         John Eddy (jpeddy@sandia.gov) (JE)
'
'     ORGANIZATION:
'
'         Sandia National Laboratories
'
'     COPYRIGHT:
'
'         This library is free software; you can redistribute it and/or
'         modify it under the terms of the GNU Lesser General Public
'         License as published by the Free Software Foundation; either
'         version 2.1 of the License, or (at your option) any later version.
'
'         This library is distributed in the hope that it will be useful,
'         but WITHOUT ANY WARRANTY; without even the implied warranty of
'         MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
'         Lesser General Public License for more details.
'
'         You should have received a copy of the GNU Lesser General Public
'         License along with this library; if not, write to the Free Software
'         Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
'         USA
'
'     VERSION:
'
'         1.0.0
'
'     CHANGES:
'
'         Fri Jan 06 10:06:49 2006 - Original Version (JE)
'
' =============================================================================
#End Region




'==============================================================================
#Region "Options"

Option Explicit On
Option Strict On

#End Region ' Options




'==============================================================================
#Region "Imports"

Imports JEGA.FrontEnd.Managed

#End Region ' Imports




'==============================================================================
' Namespace Declaration
'==============================================================================
Namespace JEGA.FrontEnd.VisualBasic



    '==========================================================================
    ' Class Declaration
    '==========================================================================
    ''' <summary>
    ''' The main class through which JEGA is used from within VB.
    ''' </summary>
    ''' 
    ''' <remarks>
    ''' This class provides the functionality necessary to run JEGA given
    ''' instructions as to how it is to be run.  Those instructions are
    ''' provided using the MProblemConfig and MAlgorithmConfig objects of the
    ''' Managed JEGA front end project.  They may be provided directly or
    ''' implementations of the VBJEGAProblemConfigLoader and
    ''' VBJEGAAlgorithmConfigLoader may be provided in their stead
    ''' respectively.
    ''' 
    ''' Prior to attempting any JEGA runs, you must initialize the library with
    ''' a call to <code>VBJEGADriver.InitializeJEGA</code>.  This should get
    ''' called once and only once.  Once that is done, the Driver can be used
    ''' in a number of ways and multiple Drivers can be operating
    ''' simultaneously (from mutliple threads if JEGA was compiled with thread
    ''' awareness).  The most common way to use it is to create one with a
    ''' MProblemConfig or VBJEGAProblemConfigLoader as the constructor argument
    ''' in the usual VB syntax.  For example:
    ''' <code>
    '''      Dim probConfig As New MProblemConfig()
    '''      probConfig.Add...
    '''      :
    '''      :
    '''      Dim driver As New VBJEGADriver(probConfig)
    ''' </code>
    ''' or
    ''' <code>
    '''      Dim probConfigLoader As New MyProbConfigLoader()
    '''      Dim driver As New VBJEGADriver(probConfigLoader)
    ''' </code>
    ''' Where MyProbConfigLoader is a class of your creation that implements
    ''' the VBJEGAProblemConfigLoader interface.  In this second case, the
    ''' VBJEGADriver constructor will create a new MProblemConfig and load
    ''' it by calling
    ''' <code>probConfigLoader.LoadConfiguration(newConfig)</code>.
    ''' 
    ''' Once the driver is created and the problem configuration is loaded, the
    ''' driver can them be used to solve problems with calls to Run.  This can
    ''' be done in one of two ways.  The first is to call the overload taking a
    ''' fully loaded MAlgorithmConfig.  The second is to call the overload
    ''' taking an MEvaluationFunctor and a VBJEGAAlgorithmConfigLoader.  For
    ''' example:
    ''' <code>
    '''      Dim evalFunctor As New MyEvalFunctor()
    '''      Dim algConfig As New MAlgorithmConfig( _
    '''           VBJEGADriver.GetEvaluator(evalFunctor) _
    '''           )
    '''      algConfig.Add...
    '''      :
    '''      :
    '''      Dim results As SolutionVector = driver.Run(algConfig)
    ''' </code>
    ''' or
    ''' <code>
    '''      Dim functor As New MyEvalFunctor()
    '''      Dim algConfigLoader as new MyAlgConfigLoader()
    '''      Dim results As SolutionVector = _
    '''          driver.Run(functor, algConfigLoader)
    ''' </code>
    ''' Where MyEvalFunctor is a class of your creation that implements the
    ''' MEvaluationFunctor interface and MyAlgConfigLoader is a class of your
    ''' creation that implements the VBJEGAAlgorithmConfigLoader interface.  In
    ''' this second case, the VBJEGADriver run method will create the
    ''' MAlgorithmConfig object needed and load it by calling
    ''' <code>algConfigLoader.LoadConfiguration(newConfig)</code>.  It will
    ''' also create the necessary evaluator object from the supplied functor
    ''' which must be done explicitely if using the first example.
    '''
    ''' You could use the driver to solve the same problem using multiple
    ''' algorithm configurations by repeating the above code over and over.  In
    ''' the second case, you would have to supply multiple algorithm
    ''' configuration loaders or create ways to change the existing one, etc.
    ''' </remarks>
    Public Class VBJEGADriver


        '======================================================================
#Region "Delegates"

#End Region ' Delegates




        '======================================================================
#Region "Events"

#End Region ' Events




        '======================================================================
#Region "Member Data"

        ''' <summary>
        ''' The problem that this driver was created to solve.
        ''' </summary>
        ''' 
        ''' <remarks>
        ''' This problem can be solved multiple times by different algorithms
        ''' by repeated calls to the Run method.
        ''' </remarks>
        Private _probConfig As MProblemConfig

#End Region ' Member Data




        '======================================================================
#Region "Public Methods"

        ''' <summary>
        ''' A convenience method that will supply a MEvaluator that will
        ''' callback to the supplied functor.
        ''' </summary>
        ''' 
        ''' <remarks>
        ''' This method will provide a new instance of an MEvaluator that
        ''' wraps the supplied functor.  A word of caution, if running multiple
        ''' algorithms from different threads, create separate instances (call
        ''' this method separately for each) to avoid multithread corruption.
        ''' </remarks>
        ''' 
        ''' <param name="functor">
        ''' The Functor that performs actual evaluations of objective functions
        ''' and constraints for a problem.
        ''' </param>
        ''' <returns>
        ''' A new MEvaluator that can be used by an MAlgorithmConfig to
        ''' help define a run of JEGA.
        ''' </returns>
        Public Shared Function _
        GetEvaluator( _
            ByVal functor As MEvaluationFunctor _
            ) As MEvaluator

            Try

                Return New MEvaluator(functor)

            Catch ex As System.Exception

                VBJEGAUtilities.DisplayError( _
                    Err, "VBJEGADriver", "GetEvaluator" _
                    )
                Return Nothing

            End Try

        End Function

        ''' <summary>
        ''' This method executes JEGA on the known problem with an algorithm
        ''' config that uses the supplied functor and is loaded by the supplied
        ''' algorithm configuration loader.
        ''' </summary>
        ''' 
        ''' <remarks>
        ''' The parameter database created for the algorithm config created
        ''' here will be an instance of the MBasicParameterDatabaseImpl.
        ''' </remarks>
        ''' 
        ''' <param name="functor">
        ''' An implementation of an MEvaluationFunctor to be called in order to
        ''' perform objective function and constraint evaluations.
        ''' </param>
        ''' <param name="algLoader">
        ''' An implementation of an VBJEGAAlgorithmConfigLoader to be used to
        ''' load a newly created MAlgorithmConfig 
        ''' </param>
        ''' <returns>
        ''' The return is an ArrayList filled with MSolution objects containing
        ''' the final solution to the problem as found by the run of JEGA.  For
        ''' a MOGA, this will be the Pareto set.  For a SOGA, the final best
        ''' solution(s).
        ''' </returns>
        Public Overridable Function _
        Run( _
           ByVal functor As MEvaluationFunctor, _
           ByVal algLoader As VBJEGAAlgorithmConfigLoader _
           ) As SolutionVector

            Try

                ' First prepare to wrap the Evaluation Delegate which is the
                ' MEvaluationFunctor that was passed in.
                Dim evaler As MEvaluator = VBJEGADriver.GetEvaluator(functor)
                GC.SuppressFinalize(evaler)

                ' Now create the configuration object that will describe the
                ' algorithm.
                Dim algConfig As New MAlgorithmConfig(evaler)
                GC.SuppressFinalize(algConfig)

                ' Load the configuration object using the supplied loader.
                algLoader.LoadConfiguration( _
                    algConfig, algConfig.GetParameterDB() _
                    )

                ' Finally, run the algorithm and return the results.
                Dim ret As SolutionVector = Me.Run(algConfig)

                GC.ReRegisterForFinalize(algConfig)
                GC.ReRegisterForFinalize(evaler)

                Return ret

            Catch ex As System.Exception

                VBJEGAUtilities.DisplayError( _
                   Err, "VBJEGADriver", _
                   "Run(MEvaluationFunctor, VBJEGAAlgorithmConfigLoader)" _
                   )
                Return Nothing

            End Try

        End Function

        ''' <summary>
        ''' This method executes JEGA on the known problem with an algorithm
        ''' config that uses the supplied functor and is loaded by the supplied
        ''' algorithm configuration loader.
        ''' </summary>
        ''' 
        ''' <remarks></remarks>
        ''' 
        ''' <param name="functor">
        ''' An implementation of an MEvaluationFunctor to be called in order to
        ''' perform objective function and constraint evaluations.
        ''' </param>
        ''' <param name="algLoader">
        ''' An implementation of an VBJEGAAlgorithmConfigLoader to be used to
        ''' load a newly created MAlgorithmConfig 
        ''' </param>
        ''' <param name="paramDB">
        ''' The parameter database to use when creating the algorithm
        ''' configuration object and into which all parameters will ultimately
        ''' be loaded.
        ''' </param>
        ''' <returns>
        ''' The return is an ArrayList filled with MSolution objects containing
        ''' the final solution to the problem as found by the run of JEGA.  For
        ''' a MOGA, this will be the Pareto set.  For a SOGA, the final best
        ''' solution(s).
        ''' </returns>
        Public Overridable Function _
        Run( _
           ByVal functor As MEvaluationFunctor, _
           ByVal algLoader As VBJEGAAlgorithmConfigLoader, _
           ByVal paramDB As MParameterDatabase _
           ) As SolutionVector

            Try

                ' First prepare to wrap the Evaluation Delegate which is the
                ' MEvaluationFunctor that was passed in.
                Dim evaler As MEvaluator = VBJEGADriver.GetEvaluator(functor)
                GC.SuppressFinalize(evaler)

                ' Now create the configuration object that will describe the
                ' algorithm.
                Dim algConfig As New MAlgorithmConfig(evaler)
                GC.SuppressFinalize(algConfig)

                ' Load the configuration object using the supplied loader.
                algLoader.LoadConfiguration(algConfig, paramDB)

                ' Finally, run the algorithm and return the results.
                Dim ret As SolutionVector = Me.Run(algConfig)

                GC.ReRegisterForFinalize(algConfig)
                GC.ReRegisterForFinalize(evaler)

                Return ret


            Catch ex As System.Exception

                VBJEGAUtilities.DisplayError( _
                   Err, "VBJEGADriver", _
                   "Run(MEvaluationFunctor, VBJEGAAlgorithmConfigLoader, " & _
                   "MParameterDatabase)" _
                   )
                Return Nothing

            End Try

        End Function

        ''' <summary>
        ''' This method executes JEGA on the known problem with the supplied
        ''' algorithm configuration object.
        ''' </summary>
        ''' 
        ''' <remarks></remarks>
        ''' 
        ''' <param name="config">
        ''' The MAlgorithmConfig describing the algorithm to be used to solve
        ''' the know problem.
        ''' </param>
        ''' <returns>
        ''' The return is an ArrayList filled with MSolution objects containing
        ''' the final solution to the problem as found by the run of JEGA.  For
        ''' a MOGA, this will be the Pareto set.  For a SOGA, the final best
        ''' solution(s).
        ''' </returns>
        Public Overridable Function _
        Run( _
            ByVal config As MAlgorithmConfig _
            ) As SolutionVector

            Try

                ' Create the application object using the known problem
                ' configuration.
                Dim FE As New MDriver(Me._probConfig)
                GC.SuppressFinalize(FE)

                ' now use the application object to run an algorithm with the
                ' supplied algorithm configuration and the contained problem
                ' configuration.
                Dim ret As SolutionVector = FE.ExecuteAlgorithm(config)

                GC.ReRegisterForFinalize(FE)
                GC.ReRegisterForFinalize(Me._probConfig)
                Return ret

            Catch ex As System.Exception

                VBJEGAUtilities.DisplayError( _
                    Err, "VBJEGADriver", "Run(MAlgorithmConfig)" _
                    )

                ' Set the return to a valid but empty array list.
                Return Nothing

            End Try

        End Function

        ''' <summary>
        ''' Method to call in order to initialize the core JEGA project.
        ''' </summary>
        ''' 
        ''' <remarks>
        ''' This method must be called once and only once prior to any calls to
        ''' the Run methods.  If you fail to do so, JEGA will abort with an
        ''' error message indicating that you have not yet called
        ''' InitializeJEGA but are trying to use JEGA anyway.
        ''' </remarks>
        ''' 
        ''' <param name="globalLogFilename">
        ''' The name of the file for JEGA's global logger to log entries into.
        ''' </param>
        ''' <param name="globalLogDefLevel">
        ''' The default level for JEGA's global log gateway to use.
        ''' </param>
        ''' <param name="rSeed">
        ''' The value to use in seeding the random number generator.  The
        ''' default value of 0 causes JEGA to use a random seed based on the
        ''' current time and the current clock values.
        ''' </param>
        ''' <returns>
        ''' The return is true if JEGA properly initializes and false
        ''' otherwise.
        ''' </returns>
        Public Shared Function _
        InitializeJEGA( _
            ByVal globalLogFilename As String, _
            ByVal globalLogDefLevel As Byte, _
            Optional ByVal rSeed As Integer = 0, _
            Optional ByVal fatalBehavior As MAlgorithmConfig.FatalBehavior = _
                MAlgorithmConfig.FatalBehavior.ABORT _
            ) As Boolean

            Try

                ' Simply call down to the managed front end application version.
                Return MDriver.InitializeJEGA( _
                    globalLogFilename, globalLogDefLevel, _
                    System.Convert.ToUInt32(rSeed), _
                    fatalBehavior _
                    )

            Catch ex As System.Exception

                VBJEGAUtilities.DisplayError( _
                    Err, "VBJEGADriver", "InitializeJEGA" _
                    )
                Return False

            End Try

        End Function

#End Region ' Public Methods




        '======================================================================
#Region "Subclass Visible Methods"

#End Region ' Subclass Visible Methods




        '======================================================================
#Region "Private Methods"

#End Region ' Private Methods




        '======================================================================
#Region "Structors"

        ''' <summary>
        ''' Constructs a VBJEGADriver and initializes the contained
        ''' MProblemConfig using the supplied problem config loader.
        ''' </summary>
        ''' 
        ''' <remarks>
        ''' This driver can only be associated with one problem configuration.
        ''' Create additional drivers to operate on different problems.
        ''' </remarks>
        ''' 
        ''' <param name="probConfigLoader">
        ''' An instance of a VBJEGAProblemConfigLoader that this constructor
        ''' will use to load up its problem configuration object.
        ''' </param>
        Public Sub _
        New( _
            ByVal probConfigLoader As VBJEGAProblemConfigLoader _
            )

            Try

                ' Initialize the MProblemConfig as a default one.
                Me._probConfig = New MProblemConfig()
                GC.SuppressFinalize(Me._probConfig)

                ' Now load it up using the supplied VBJEGAProblemConfigLoader
                probConfigLoader.LoadConfiguration(Me._probConfig)

            Catch ex As System.Exception

                VBJEGAUtilities.DisplayError( _
                    Err, "VBJEGADriver", "InitializeJEGA" _
                    )

            End Try

        End Sub

        ''' <summary>
        ''' Constructs a VBJEGADriver and initializes the contained
        ''' MProblemConfig to be the one supplied.
        ''' </summary>
        ''' 
        ''' <remarks></remarks>
        ''' 
        ''' <param name="probConfig">
        ''' The MProblemConfig that is to be known by this driver.
        ''' </param>
        Public Sub _
        New( _
            ByVal probConfig As MProblemConfig _
            )

            MyBase.New()

            ' Simply adopt the supplied problem configuration as our own.
            Me._probConfig = probConfig
            GC.SuppressFinalize(Me._probConfig)

        End Sub

#End Region ' Structors


    End Class ' VBJEGADriver

End Namespace ' JEGA.FrontEnd.VisualBasic