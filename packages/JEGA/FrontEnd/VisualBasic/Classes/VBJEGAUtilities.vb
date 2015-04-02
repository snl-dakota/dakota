' =============================================================================
#Region "File Preamble"
' =============================================================================
'     PROJECT:
'
'         John Eddy's Genetic Algorithms (JEGA) Visual Basic Front End
'
'     CONTENTS:
'
'         Declaration of Class VBJEGAUtilities.
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
    ''' A class to house general utility functions for the JEGA VB front end.
    ''' </summary>
    ''' 
    ''' <remarks>
    ''' This class has no specific charter other than to house useful methods
    ''' that don't have another home.
    ''' </remarks>
    Public NotInheritable Class VBJEGAUtilities


        '======================================================================
#Region "Delegates"

#End Region ' Delegates




        '======================================================================
#Region "Events"

#End Region ' Events




        '======================================================================
#Region "Member Data"

#End Region ' Member Data




        '======================================================================
#Region "Public Methods"

        ''' <summary>
        ''' Prints each of the MSolution objects contained in
        ''' <paramref>sols</paramref> to the supplied
        ''' <paramref>stream</paramref> using the
        ''' <code>VBJEGAUtilities.WriteSolution</code> method.
        ''' </summary>
        ''' 
        ''' <remarks>
        ''' See <code>VBJEGAUtilities.WriteSolution</code> for formatting
        ''' details.
        ''' </remarks>
        ''' 
        ''' <param name="sols">
        ''' The collection of MSolution objects to write to the
        ''' <paramref>stream</paramref>
        ''' </param>
        ''' <param name="stream">
        ''' The place to put the text representation of each MSolution in
        ''' <paramref>sols</paramref>.
        ''' </param>
        Public Shared Sub _
        WriteSolutions( _
            ByVal sols As SolutionVector, _
            ByVal stream As System.IO.TextWriter _
            )

            Try

                ' Just call WriteSolution for each entry in sols and put a
                ' new line after each.
                For Each sol As MSolution In sols
                    WriteSolution(sol, stream)
                    stream.WriteLine()
                Next sol

            Catch ex As System.Exception

                VBJEGAUtilities.DisplayError( _
                    Err, "VBJEGAUtilities", "WriteSolutions" _
                    )

            End Try

        End Sub

        ''' <summary>
        ''' Writes the text representation of the supplied MSolution object
        ''' into the supplied TextWriter.
        ''' </summary>
        ''' 
        ''' <remarks>
        ''' The solution is written in the following format:
        ''' dv1 dv2 dv3 ... of1 of2 of3 ... cn1 cn1 cn3 ...
        ''' where the whitespace between entries is a tab character.  The
        ''' objectives and constraints (of# and cn#) are only written if the
        ''' Solution is evaluated and is well conditioned.
        ''' </remarks>
        ''' 
        ''' <param name="sol">
        ''' The MSolution to write into <paramref>stream</paramref>.
        ''' </param>
        ''' <param name="stream">
        ''' The TextWriter into which to write the text representation of
        ''' <paramref>sol</paramref>.
        ''' </param>
        Public Shared Sub _
        WriteSolution( _
            ByVal sol As MSolution, _
            ByVal stream As System.IO.TextWriter _
            )

            Try

                ' Store ndv so we don't have to constantly call sol.GetNDV().
                Dim ndv As Integer = System.Convert.ToInt32(sol.GetNDV())

                If (ndv > 0) Then

                    ' Put out all but the last dv with a tab after it.
                    For var As Integer = 0 To (ndv - 2)
                        stream.Write( _
                            sol.GetVariable(CUInt(var)).ToString & Chr(9) _
                            )
                    Next var

                    ' now put out the last design variable without a tab
                    ' so that if the responses don't get written, there is
                    ' no hanging tab.
                    stream.Write(sol.GetVariable(CUInt(ndv - 1)))

                End If

                ' only print out responses if the Solution has been
                ' evaluated and is well conditioned.
                If sol.IsEvaluated() AndAlso Not sol.IsIllconditioned() Then

                    ' Store nof and ncn so we don't have to constantly call
                    ' sol.GetNOF() and sol.GetNCN().
                    Dim nof As Integer = System.Convert.ToInt32(sol.GetNOF())
                    Dim ncn As Integer = System.Convert.ToInt32(sol.GetNCN())

                    ' put out each objective after a tab character
                    If (nof > 0) Then
                        For obj As Integer = 0 To (nof - 1)
                            stream.Write( _
                                ControlChars.Tab & _
                                sol.GetObjective(CUInt(obj)).ToString _
                                )
                        Next obj
                    End If

                    ' now do the same for any constraints.
                    If (ncn > 0) Then
                        For con As Integer = 0 To (ncn - 1)
                            stream.Write( _
                                ControlChars.Tab & _
                                sol.GetConstraint(CUInt(con)).ToString _
                                )
                        Next con
                    End If

                End If

            Catch ex As System.Exception

                VBJEGAUtilities.DisplayError( _
                    Err, "VBJEGAUtilities", "WriteSolution" _
                    )

            End Try

        End Sub

        ''' <summary>
        ''' Call this method to display an error in JEGA VB front end default
        ''' format.
        ''' </summary>
        ''' 
        ''' <remarks>
        ''' The default format is to write:
        '''
        ''' In m.f Error #e.Number raised with description e.Description.  msg
        ''' </remarks>
        ''' 
        ''' <param name="e">
        ''' The error object describing the error.
        ''' </param>
        ''' <param name="m">
        ''' The module in which the error occurred.
        ''' </param>
        ''' <param name="f">
        ''' The function name in which the error occurred (or sub, property,
        ''' etc.).
        ''' </param>
        ''' <param name="msg">
        ''' An optional additional message to send to the user.
        ''' </param>
        Public Shared Sub _
        DisplayError( _
            ByVal e As ErrObject, _
            ByVal m As String, _
            ByVal f As String, _
            Optional ByVal msg As String = "" _
            )

            MsgBox("In " & e.Source & "." & m & "." & f & ": Error #" & _
                   e.Number.ToString & " raised with description """ & _
                   e.Description & """.  " & msg)

        End Sub

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
        ''' Default constructs a VBJEGAUtilities.
        ''' </summary>
        ''' 
        ''' <remarks>
        ''' This class has only shared members and thus is not meant to be
        ''' instantiated.
        ''' </remarks>
        Private Sub _
        New( _
            )

            MyBase.New()

        End Sub

#End Region ' Structors


    End Class ' VBJEGAUtilities

End Namespace ' JEGA.FrontEnd.VisualBasic
