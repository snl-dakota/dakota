/*******************************************************************************
 * Sandia Analysis Workbench Integration Framework (SAW)
 * Copyright 2016 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * 
 * This software is distributed under the Eclipse Public License.
 * For more information see the files copyright.txt and license.txt
 * included with the software.
 ******************************************************************************/
// Generated from NIDR.g4 by ANTLR 4.1
package gov.sandia.dart.dakota.nidr.antlr;
import org.antlr.v4.runtime.misc.NotNull;
import org.antlr.v4.runtime.tree.ParseTreeListener;

/**
 * This interface defines a complete listener for a parse tree produced by
 * {@link NIDRParser}.
 */
public interface NIDRListener extends ParseTreeListener {
	/**
	 * Enter a parse tree produced by {@link NIDRParser#gSingle}.
	 * @param ctx the parse tree
	 */
	void enterGSingle(@NotNull NIDRParser.GSingleContext ctx);
	/**
	 * Exit a parse tree produced by {@link NIDRParser#gSingle}.
	 * @param ctx the parse tree
	 */
	void exitGSingle(@NotNull NIDRParser.GSingleContext ctx);

	/**
	 * Enter a parse tree produced by {@link NIDRParser#requiredGroup}.
	 * @param ctx the parse tree
	 */
	void enterRequiredGroup(@NotNull NIDRParser.RequiredGroupContext ctx);
	/**
	 * Exit a parse tree produced by {@link NIDRParser#requiredGroup}.
	 * @param ctx the parse tree
	 */
	void exitRequiredGroup(@NotNull NIDRParser.RequiredGroupContext ctx);

	/**
	 * Enter a parse tree produced by {@link NIDRParser#toplevel}.
	 * @param ctx the parse tree
	 */
	void enterToplevel(@NotNull NIDRParser.ToplevelContext ctx);
	/**
	 * Exit a parse tree produced by {@link NIDRParser#toplevel}.
	 * @param ctx the parse tree
	 */
	void exitToplevel(@NotNull NIDRParser.ToplevelContext ctx);

	/**
	 * Enter a parse tree produced by {@link NIDRParser#input}.
	 * @param ctx the parse tree
	 */
	void enterInput(@NotNull NIDRParser.InputContext ctx);
	/**
	 * Exit a parse tree produced by {@link NIDRParser#input}.
	 * @param ctx the parse tree
	 */
	void exitInput(@NotNull NIDRParser.InputContext ctx);

	/**
	 * Enter a parse tree produced by {@link NIDRParser#gRequiredGroup}.
	 * @param ctx the parse tree
	 */
	void enterGRequiredGroup(@NotNull NIDRParser.GRequiredGroupContext ctx);
	/**
	 * Exit a parse tree produced by {@link NIDRParser#gRequiredGroup}.
	 * @param ctx the parse tree
	 */
	void exitGRequiredGroup(@NotNull NIDRParser.GRequiredGroupContext ctx);

	/**
	 * Enter a parse tree produced by {@link NIDRParser#single}.
	 * @param ctx the parse tree
	 */
	void enterSingle(@NotNull NIDRParser.SingleContext ctx);
	/**
	 * Exit a parse tree produced by {@link NIDRParser#single}.
	 * @param ctx the parse tree
	 */
	void exitSingle(@NotNull NIDRParser.SingleContext ctx);

	/**
	 * Enter a parse tree produced by {@link NIDRParser#orGroup}.
	 * @param ctx the parse tree
	 */
	void enterOrGroup(@NotNull NIDRParser.OrGroupContext ctx);
	/**
	 * Exit a parse tree produced by {@link NIDRParser#orGroup}.
	 * @param ctx the parse tree
	 */
	void exitOrGroup(@NotNull NIDRParser.OrGroupContext ctx);

	/**
	 * Enter a parse tree produced by {@link NIDRParser#optionalGroup}.
	 * @param ctx the parse tree
	 */
	void enterOptionalGroup(@NotNull NIDRParser.OptionalGroupContext ctx);
	/**
	 * Exit a parse tree produced by {@link NIDRParser#optionalGroup}.
	 * @param ctx the parse tree
	 */
	void exitOptionalGroup(@NotNull NIDRParser.OptionalGroupContext ctx);

	/**
	 * Enter a parse tree produced by {@link NIDRParser#command}.
	 * @param ctx the parse tree
	 */
	void enterCommand(@NotNull NIDRParser.CommandContext ctx);
	/**
	 * Exit a parse tree produced by {@link NIDRParser#command}.
	 * @param ctx the parse tree
	 */
	void exitCommand(@NotNull NIDRParser.CommandContext ctx);

	/**
	 * Enter a parse tree produced by {@link NIDRParser#listWithHead}.
	 * @param ctx the parse tree
	 */
	void enterListWithHead(@NotNull NIDRParser.ListWithHeadContext ctx);
	/**
	 * Exit a parse tree produced by {@link NIDRParser#listWithHead}.
	 * @param ctx the parse tree
	 */
	void exitListWithHead(@NotNull NIDRParser.ListWithHeadContext ctx);

	/**
	 * Enter a parse tree produced by {@link NIDRParser#gOptionalGroup}.
	 * @param ctx the parse tree
	 */
	void enterGOptionalGroup(@NotNull NIDRParser.GOptionalGroupContext ctx);
	/**
	 * Exit a parse tree produced by {@link NIDRParser#gOptionalGroup}.
	 * @param ctx the parse tree
	 */
	void exitGOptionalGroup(@NotNull NIDRParser.GOptionalGroupContext ctx);

	/**
	 * Enter a parse tree produced by {@link NIDRParser#listWithNoHead}.
	 * @param ctx the parse tree
	 */
	void enterListWithNoHead(@NotNull NIDRParser.ListWithNoHeadContext ctx);
	/**
	 * Exit a parse tree produced by {@link NIDRParser#listWithNoHead}.
	 * @param ctx the parse tree
	 */
	void exitListWithNoHead(@NotNull NIDRParser.ListWithNoHeadContext ctx);
}
