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
import org.antlr.v4.runtime.tree.ParseTreeVisitor;

/**
 * This interface defines a complete generic visitor for a parse tree produced
 * by {@link NIDRParser}.
 *
 * @param <T> The return type of the visit operation. Use {@link Void} for
 * operations with no return type.
 */
public interface NIDRVisitor<T> extends ParseTreeVisitor<T> {
	/**
	 * Visit a parse tree produced by {@link NIDRParser#gSingle}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitGSingle(@NotNull NIDRParser.GSingleContext ctx);

	/**
	 * Visit a parse tree produced by {@link NIDRParser#requiredGroup}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRequiredGroup(@NotNull NIDRParser.RequiredGroupContext ctx);

	/**
	 * Visit a parse tree produced by {@link NIDRParser#toplevel}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitToplevel(@NotNull NIDRParser.ToplevelContext ctx);

	/**
	 * Visit a parse tree produced by {@link NIDRParser#input}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitInput(@NotNull NIDRParser.InputContext ctx);

	/**
	 * Visit a parse tree produced by {@link NIDRParser#gRequiredGroup}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitGRequiredGroup(@NotNull NIDRParser.GRequiredGroupContext ctx);

	/**
	 * Visit a parse tree produced by {@link NIDRParser#single}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSingle(@NotNull NIDRParser.SingleContext ctx);

	/**
	 * Visit a parse tree produced by {@link NIDRParser#orGroup}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitOrGroup(@NotNull NIDRParser.OrGroupContext ctx);

	/**
	 * Visit a parse tree produced by {@link NIDRParser#optionalGroup}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitOptionalGroup(@NotNull NIDRParser.OptionalGroupContext ctx);

	/**
	 * Visit a parse tree produced by {@link NIDRParser#command}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitCommand(@NotNull NIDRParser.CommandContext ctx);

	/**
	 * Visit a parse tree produced by {@link NIDRParser#listWithHead}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitListWithHead(@NotNull NIDRParser.ListWithHeadContext ctx);

	/**
	 * Visit a parse tree produced by {@link NIDRParser#gOptionalGroup}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitGOptionalGroup(@NotNull NIDRParser.GOptionalGroupContext ctx);

	/**
	 * Visit a parse tree produced by {@link NIDRParser#listWithNoHead}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitListWithNoHead(@NotNull NIDRParser.ListWithNoHeadContext ctx);
}
