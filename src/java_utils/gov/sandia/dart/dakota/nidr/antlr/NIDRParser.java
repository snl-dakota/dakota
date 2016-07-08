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
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.misc.*;
import org.antlr.v4.runtime.tree.*;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast"})
public class NIDRParser extends Parser {
	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		OPENB=1, CLOSEB=2, OPENP=3, CLOSEP=4, STRING=5, LENSPEC=6, ALIAS=7, KEYWORD=8, 
		SYMBOL=9, MACRO=10, OP=11, OR=12, NUMBER=13, COMMENT=14, WS=15;
	public static final String[] tokenNames = {
		"<INVALID>", "'['", "']'", "'('", "')'", "STRING", "LENSPEC", "'ALIAS'", 
		"KEYWORD", "SYMBOL", "MACRO", "OP", "'|'", "NUMBER", "COMMENT", "WS"
	};
	public static final int
		RULE_input = 0, RULE_toplevel = 1, RULE_commands = 2, RULE_command = 3, 
		RULE_group = 4;
	public static final String[] ruleNames = {
		"input", "toplevel", "commands", "command", "group"
	};

	@Override
	public String getGrammarFileName() { return "NIDR.g4"; }

	@Override
	public String[] getTokenNames() { return tokenNames; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public ATN getATN() { return _ATN; }

	public NIDRParser(TokenStream input) {
		super(input);
		_interp = new ParserATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}
	public static class InputContext extends ParserRuleContext {
		public ToplevelContext toplevel(int i) {
			return getRuleContext(ToplevelContext.class,i);
		}
		public List<ToplevelContext> toplevel() {
			return getRuleContexts(ToplevelContext.class);
		}
		public InputContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_input; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).enterInput(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).exitInput(this);
		}
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof NIDRVisitor ) return ((NIDRVisitor<? extends T>)visitor).visitInput(this);
			else return visitor.visitChildren(this);
		}
	}

	public final InputContext input() throws RecognitionException {
		InputContext _localctx = new InputContext(_ctx, getState());
		enterRule(_localctx, 0, RULE_input);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(11); 
			_errHandler.sync(this);
			_la = _input.LA(1);
			do {
				{
				{
				setState(10); toplevel();
				}
				}
				setState(13); 
				_errHandler.sync(this);
				_la = _input.LA(1);
			} while ( _la==KEYWORD );
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ToplevelContext extends ParserRuleContext {
		public TerminalNode MACRO() { return getToken(NIDRParser.MACRO, 0); }
		public List<CommandContext> command() {
			return getRuleContexts(CommandContext.class);
		}
		public TerminalNode SYMBOL() { return getToken(NIDRParser.SYMBOL, 0); }
		public CommandsContext commands(int i) {
			return getRuleContext(CommandsContext.class,i);
		}
		public TerminalNode KEYWORD() { return getToken(NIDRParser.KEYWORD, 0); }
		public CommandContext command(int i) {
			return getRuleContext(CommandContext.class,i);
		}
		public List<CommandsContext> commands() {
			return getRuleContexts(CommandsContext.class);
		}
		public ToplevelContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_toplevel; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).enterToplevel(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).exitToplevel(this);
		}
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof NIDRVisitor ) return ((NIDRVisitor<? extends T>)visitor).visitToplevel(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ToplevelContext toplevel() throws RecognitionException {
		ToplevelContext _localctx = new ToplevelContext(_ctx, getState());
		enterRule(_localctx, 2, RULE_toplevel);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(15); match(KEYWORD);
			setState(16); match(SYMBOL);
			setState(17); match(MACRO);
			setState(22);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while ((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << OPENB) | (1L << OPENP) | (1L << SYMBOL))) != 0)) {
				{
				setState(20);
				switch ( getInterpreter().adaptivePredict(_input,1,_ctx) ) {
				case 1:
					{
					setState(18); command();
					}
					break;

				case 2:
					{
					setState(19); commands(0);
					}
					break;
				}
				}
				setState(24);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class CommandsContext extends ParserRuleContext {
		public int _p;
		public CommandsContext(ParserRuleContext parent, int invokingState) { super(parent, invokingState); }
		public CommandsContext(ParserRuleContext parent, int invokingState, int _p) {
			super(parent, invokingState);
			this._p = _p;
		}
		@Override public int getRuleIndex() { return RULE_commands; }
	 
		public CommandsContext() { }
		public void copyFrom(CommandsContext ctx) {
			super.copyFrom(ctx);
			this._p = ctx._p;
		}
	}
	public static class RequiredGroupContext extends CommandsContext {
		public TerminalNode CLOSEP() { return getToken(NIDRParser.CLOSEP, 0); }
		public TerminalNode OPENP() { return getToken(NIDRParser.OPENP, 0); }
		public CommandsContext commands() {
			return getRuleContext(CommandsContext.class,0);
		}
		public RequiredGroupContext(CommandsContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).enterRequiredGroup(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).exitRequiredGroup(this);
		}
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof NIDRVisitor ) return ((NIDRVisitor<? extends T>)visitor).visitRequiredGroup(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class SingleContext extends CommandsContext {
		public CommandContext command() {
			return getRuleContext(CommandContext.class,0);
		}
		public SingleContext(CommandsContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).enterSingle(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).exitSingle(this);
		}
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof NIDRVisitor ) return ((NIDRVisitor<? extends T>)visitor).visitSingle(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class OrGroupContext extends CommandsContext {
		public GroupContext group(int i) {
			return getRuleContext(GroupContext.class,i);
		}
		public List<GroupContext> group() {
			return getRuleContexts(GroupContext.class);
		}
		public OrGroupContext(CommandsContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).enterOrGroup(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).exitOrGroup(this);
		}
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof NIDRVisitor ) return ((NIDRVisitor<? extends T>)visitor).visitOrGroup(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class OptionalGroupContext extends CommandsContext {
		public TerminalNode OPENB() { return getToken(NIDRParser.OPENB, 0); }
		public TerminalNode CLOSEB() { return getToken(NIDRParser.CLOSEB, 0); }
		public CommandsContext commands() {
			return getRuleContext(CommandsContext.class,0);
		}
		public OptionalGroupContext(CommandsContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).enterOptionalGroup(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).exitOptionalGroup(this);
		}
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof NIDRVisitor ) return ((NIDRVisitor<? extends T>)visitor).visitOptionalGroup(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ListWithHeadContext extends CommandsContext {
		public CommandContext command() {
			return getRuleContext(CommandContext.class,0);
		}
		public CommandsContext commands() {
			return getRuleContext(CommandsContext.class,0);
		}
		public ListWithHeadContext(CommandsContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).enterListWithHead(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).exitListWithHead(this);
		}
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof NIDRVisitor ) return ((NIDRVisitor<? extends T>)visitor).visitListWithHead(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ListWithNoHeadContext extends CommandsContext {
		public CommandsContext commands(int i) {
			return getRuleContext(CommandsContext.class,i);
		}
		public List<CommandsContext> commands() {
			return getRuleContexts(CommandsContext.class);
		}
		public ListWithNoHeadContext(CommandsContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).enterListWithNoHead(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).exitListWithNoHead(this);
		}
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof NIDRVisitor ) return ((NIDRVisitor<? extends T>)visitor).visitListWithNoHead(this);
			else return visitor.visitChildren(this);
		}
	}

	public final CommandsContext commands(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		CommandsContext _localctx = new CommandsContext(_ctx, _parentState, _p);
		CommandsContext _prevctx = _localctx;
		int _startState = 4;
		enterRecursionRule(_localctx, RULE_commands);
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(45);
			switch ( getInterpreter().adaptivePredict(_input,4,_ctx) ) {
			case 1:
				{
				_localctx = new ListWithHeadContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(26); command();
				setState(27); commands(5);
				}
				break;

			case 2:
				{
				_localctx = new SingleContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(29); command();
				}
				break;

			case 3:
				{
				_localctx = new OrGroupContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(30); group();
				setState(33); 
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,3,_ctx);
				do {
					switch (_alt) {
					case 1:
						{
						{
						setState(31); match(OR);
						setState(32); group();
						}
						}
						break;
					default:
						throw new NoViableAltException(this);
					}
					setState(35); 
					_errHandler.sync(this);
					_alt = getInterpreter().adaptivePredict(_input,3,_ctx);
				} while ( _alt!=2 && _alt!=-1 );
				}
				break;

			case 4:
				{
				_localctx = new OptionalGroupContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(37); match(OPENB);
				setState(38); commands(0);
				setState(39); match(CLOSEB);
				}
				break;

			case 5:
				{
				_localctx = new RequiredGroupContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(41); match(OPENP);
				setState(42); commands(0);
				setState(43); match(CLOSEP);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(51);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,5,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new ListWithNoHeadContext(new CommandsContext(_parentctx, _parentState, _p));
					pushNewRecursionContext(_localctx, _startState, RULE_commands);
					setState(47);
					if (!(4 >= _localctx._p)) throw new FailedPredicateException(this, "4 >= $_p");
					setState(48); commands(0);
					}
					} 
				}
				setState(53);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,5,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	public static class CommandContext extends ParserRuleContext {
		public TerminalNode MACRO() { return getToken(NIDRParser.MACRO, 0); }
		public TerminalNode OP() { return getToken(NIDRParser.OP, 0); }
		public List<TerminalNode> SYMBOL() { return getTokens(NIDRParser.SYMBOL); }
		public TerminalNode SYMBOL(int i) {
			return getToken(NIDRParser.SYMBOL, i);
		}
		public TerminalNode LENSPEC() { return getToken(NIDRParser.LENSPEC, 0); }
		public TerminalNode NUMBER() { return getToken(NIDRParser.NUMBER, 0); }
		public List<TerminalNode> ALIAS() { return getTokens(NIDRParser.ALIAS); }
		public TerminalNode ALIAS(int i) {
			return getToken(NIDRParser.ALIAS, i);
		}
		public CommandContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_command; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).enterCommand(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).exitCommand(this);
		}
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof NIDRVisitor ) return ((NIDRVisitor<? extends T>)visitor).visitCommand(this);
			else return visitor.visitChildren(this);
		}
	}

	public final CommandContext command() throws RecognitionException {
		CommandContext _localctx = new CommandContext(_ctx, getState());
		enterRule(_localctx, 6, RULE_command);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(54); match(SYMBOL);
			setState(59);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==ALIAS) {
				{
				{
				setState(55); match(ALIAS);
				setState(56); match(SYMBOL);
				}
				}
				setState(61);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(69);
			_la = _input.LA(1);
			if (_la==SYMBOL) {
				{
				setState(62); match(SYMBOL);
				setState(67);
				switch (_input.LA(1)) {
				case OP:
					{
					{
					setState(63); match(OP);
					setState(64); match(NUMBER);
					}
					}
					break;
				case LENSPEC:
					{
					{
					setState(65); match(LENSPEC);
					setState(66); match(SYMBOL);
					}
					}
					break;
				case MACRO:
					break;
				default:
					throw new NoViableAltException(this);
				}
				}
			}

			setState(71); match(MACRO);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class GroupContext extends ParserRuleContext {
		public GroupContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_group; }
	 
		public GroupContext() { }
		public void copyFrom(GroupContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class GSingleContext extends GroupContext {
		public CommandContext command() {
			return getRuleContext(CommandContext.class,0);
		}
		public GSingleContext(GroupContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).enterGSingle(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).exitGSingle(this);
		}
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof NIDRVisitor ) return ((NIDRVisitor<? extends T>)visitor).visitGSingle(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class GRequiredGroupContext extends GroupContext {
		public TerminalNode CLOSEP() { return getToken(NIDRParser.CLOSEP, 0); }
		public TerminalNode OPENP() { return getToken(NIDRParser.OPENP, 0); }
		public CommandsContext commands() {
			return getRuleContext(CommandsContext.class,0);
		}
		public GRequiredGroupContext(GroupContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).enterGRequiredGroup(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).exitGRequiredGroup(this);
		}
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof NIDRVisitor ) return ((NIDRVisitor<? extends T>)visitor).visitGRequiredGroup(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class GOptionalGroupContext extends GroupContext {
		public TerminalNode OPENB() { return getToken(NIDRParser.OPENB, 0); }
		public TerminalNode CLOSEB() { return getToken(NIDRParser.CLOSEB, 0); }
		public CommandsContext commands() {
			return getRuleContext(CommandsContext.class,0);
		}
		public GOptionalGroupContext(GroupContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).enterGOptionalGroup(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof NIDRListener ) ((NIDRListener)listener).exitGOptionalGroup(this);
		}
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof NIDRVisitor ) return ((NIDRVisitor<? extends T>)visitor).visitGOptionalGroup(this);
			else return visitor.visitChildren(this);
		}
	}

	public final GroupContext group() throws RecognitionException {
		GroupContext _localctx = new GroupContext(_ctx, getState());
		enterRule(_localctx, 8, RULE_group);
		try {
			setState(82);
			switch (_input.LA(1)) {
			case SYMBOL:
				_localctx = new GSingleContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(73); command();
				}
				break;
			case OPENB:
				_localctx = new GOptionalGroupContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(74); match(OPENB);
				setState(75); commands(0);
				setState(76); match(CLOSEB);
				}
				break;
			case OPENP:
				_localctx = new GRequiredGroupContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(78); match(OPENP);
				setState(79); commands(0);
				setState(80); match(CLOSEP);
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public boolean sempred(RuleContext _localctx, int ruleIndex, int predIndex) {
		switch (ruleIndex) {
		case 2: return commands_sempred((CommandsContext)_localctx, predIndex);
		}
		return true;
	}
	private boolean commands_sempred(CommandsContext _localctx, int predIndex) {
		switch (predIndex) {
		case 0: return 4 >= _localctx._p;
		}
		return true;
	}

	public static final String _serializedATN =
		"\3\uacf5\uee8c\u4f5d\u8b0d\u4a45\u78bd\u1b2f\u3378\3\21W\4\2\t\2\4\3\t"+
		"\3\4\4\t\4\4\5\t\5\4\6\t\6\3\2\6\2\16\n\2\r\2\16\2\17\3\3\3\3\3\3\3\3"+
		"\3\3\7\3\27\n\3\f\3\16\3\32\13\3\3\4\3\4\3\4\3\4\3\4\3\4\3\4\3\4\6\4$"+
		"\n\4\r\4\16\4%\3\4\3\4\3\4\3\4\3\4\3\4\3\4\3\4\5\4\60\n\4\3\4\3\4\7\4"+
		"\64\n\4\f\4\16\4\67\13\4\3\5\3\5\3\5\7\5<\n\5\f\5\16\5?\13\5\3\5\3\5\3"+
		"\5\3\5\3\5\5\5F\n\5\5\5H\n\5\3\5\3\5\3\6\3\6\3\6\3\6\3\6\3\6\3\6\3\6\3"+
		"\6\5\6U\n\6\3\6\2\7\2\4\6\b\n\2\2`\2\r\3\2\2\2\4\21\3\2\2\2\6/\3\2\2\2"+
		"\b8\3\2\2\2\nT\3\2\2\2\f\16\5\4\3\2\r\f\3\2\2\2\16\17\3\2\2\2\17\r\3\2"+
		"\2\2\17\20\3\2\2\2\20\3\3\2\2\2\21\22\7\n\2\2\22\23\7\13\2\2\23\30\7\f"+
		"\2\2\24\27\5\b\5\2\25\27\5\6\4\2\26\24\3\2\2\2\26\25\3\2\2\2\27\32\3\2"+
		"\2\2\30\26\3\2\2\2\30\31\3\2\2\2\31\5\3\2\2\2\32\30\3\2\2\2\33\34\b\4"+
		"\1\2\34\35\5\b\5\2\35\36\5\6\4\2\36\60\3\2\2\2\37\60\5\b\5\2 #\5\n\6\2"+
		"!\"\7\16\2\2\"$\5\n\6\2#!\3\2\2\2$%\3\2\2\2%#\3\2\2\2%&\3\2\2\2&\60\3"+
		"\2\2\2\'(\7\3\2\2()\5\6\4\2)*\7\4\2\2*\60\3\2\2\2+,\7\5\2\2,-\5\6\4\2"+
		"-.\7\6\2\2.\60\3\2\2\2/\33\3\2\2\2/\37\3\2\2\2/ \3\2\2\2/\'\3\2\2\2/+"+
		"\3\2\2\2\60\65\3\2\2\2\61\62\6\4\2\3\62\64\5\6\4\2\63\61\3\2\2\2\64\67"+
		"\3\2\2\2\65\63\3\2\2\2\65\66\3\2\2\2\66\7\3\2\2\2\67\65\3\2\2\28=\7\13"+
		"\2\29:\7\t\2\2:<\7\13\2\2;9\3\2\2\2<?\3\2\2\2=;\3\2\2\2=>\3\2\2\2>G\3"+
		"\2\2\2?=\3\2\2\2@E\7\13\2\2AB\7\r\2\2BF\7\17\2\2CD\7\b\2\2DF\7\13\2\2"+
		"EA\3\2\2\2EC\3\2\2\2EF\3\2\2\2FH\3\2\2\2G@\3\2\2\2GH\3\2\2\2HI\3\2\2\2"+
		"IJ\7\f\2\2J\t\3\2\2\2KU\5\b\5\2LM\7\3\2\2MN\5\6\4\2NO\7\4\2\2OU\3\2\2"+
		"\2PQ\7\5\2\2QR\5\6\4\2RS\7\6\2\2SU\3\2\2\2TK\3\2\2\2TL\3\2\2\2TP\3\2\2"+
		"\2U\13\3\2\2\2\f\17\26\30%/\65=EGT";
	public static final ATN _ATN =
		ATNSimulator.deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}
