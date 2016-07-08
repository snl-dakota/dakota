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
import org.antlr.v4.runtime.Lexer;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.misc.*;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast"})
public class NIDRLexer extends Lexer {
	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		OPENB=1, CLOSEB=2, OPENP=3, CLOSEP=4, STRING=5, LENSPEC=6, ALIAS=7, KEYWORD=8, 
		SYMBOL=9, MACRO=10, OP=11, OR=12, NUMBER=13, COMMENT=14, WS=15;
	public static String[] modeNames = {
		"DEFAULT_MODE"
	};

	public static final String[] tokenNames = {
		"<INVALID>",
		"'['", "']'", "'('", "')'", "STRING", "LENSPEC", "'ALIAS'", "KEYWORD", 
		"SYMBOL", "MACRO", "OP", "'|'", "NUMBER", "COMMENT", "WS"
	};
	public static final String[] ruleNames = {
		"OPENB", "CLOSEB", "OPENP", "CLOSEP", "STRING", "LENSPEC", "ALIAS", "KEYWORD", 
		"SYMBOL", "MACRO", "OP", "OR", "NUMBER", "INT", "MANTISSA", "EXP", "COMMENT", 
		"WS"
	};


	public NIDRLexer(CharStream input) {
		super(input);
		_interp = new LexerATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}

	@Override
	public String getGrammarFileName() { return "NIDR.g4"; }

	@Override
	public String[] getTokenNames() { return tokenNames; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String[] getModeNames() { return modeNames; }

	@Override
	public ATN getATN() { return _ATN; }

	@Override
	public void action(RuleContext _localctx, int ruleIndex, int actionIndex) {
		switch (ruleIndex) {
		case 16: COMMENT_action((RuleContext)_localctx, actionIndex); break;

		case 17: WS_action((RuleContext)_localctx, actionIndex); break;
		}
	}
	private void WS_action(RuleContext _localctx, int actionIndex) {
		switch (actionIndex) {
		case 1: skip();  break;
		}
	}
	private void COMMENT_action(RuleContext _localctx, int actionIndex) {
		switch (actionIndex) {
		case 0: skip();  break;
		}
	}

	public static final String _serializedATN =
		"\3\uacf5\uee8c\u4f5d\u8b0d\u4a45\u78bd\u1b2f\u3378\2\21\u00ad\b\1\4\2"+
		"\t\2\4\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7\4\b\t\b\4\t\t\t\4\n\t\n\4"+
		"\13\t\13\4\f\t\f\4\r\t\r\4\16\t\16\4\17\t\17\4\20\t\20\4\21\t\21\4\22"+
		"\t\22\4\23\t\23\3\2\3\2\3\3\3\3\3\4\3\4\3\5\3\5\3\6\3\6\7\6\62\n\6\f\6"+
		"\16\6\65\13\6\3\6\3\6\3\7\3\7\3\7\3\7\3\7\3\7\3\7\5\7@\n\7\3\b\3\b\3\b"+
		"\3\b\3\b\3\b\3\t\3\t\3\t\3\t\3\t\3\t\3\t\3\t\3\t\3\t\7\tR\n\t\f\t\16\t"+
		"U\13\t\3\n\3\n\7\nY\n\n\f\n\16\n\\\13\n\3\13\3\13\7\13`\n\13\f\13\16\13"+
		"c\13\13\3\13\3\13\3\f\3\f\3\f\5\fj\n\f\3\r\3\r\3\16\5\16o\n\16\3\16\3"+
		"\16\3\16\3\16\5\16u\n\16\3\16\5\16x\n\16\3\16\3\16\3\16\3\16\5\16~\n\16"+
		"\3\16\5\16\u0081\n\16\3\17\3\17\3\17\7\17\u0086\n\17\f\17\16\17\u0089"+
		"\13\17\5\17\u008b\n\17\3\20\7\20\u008e\n\20\f\20\16\20\u0091\13\20\3\21"+
		"\3\21\5\21\u0095\n\21\3\21\3\21\3\22\3\22\7\22\u009b\n\22\f\22\16\22\u009e"+
		"\13\22\3\22\5\22\u00a1\n\22\3\22\3\22\3\22\3\22\3\23\6\23\u00a8\n\23\r"+
		"\23\16\23\u00a9\3\23\3\23\3\u009c\24\3\3\1\5\4\1\7\5\1\t\6\1\13\7\1\r"+
		"\b\1\17\t\1\21\n\1\23\13\1\25\f\1\27\r\1\31\16\1\33\17\1\35\2\1\37\2\1"+
		"!\2\1#\20\2%\21\3\3\2\13\3\2$$\3\2\62;\5\2C\\aac|\6\2\62;C\\aac|\3\2\177"+
		"\177\3\2\63;\4\2GGgg\4\2--//\5\2\13\f\17\17\"\"\u00bc\2\3\3\2\2\2\2\5"+
		"\3\2\2\2\2\7\3\2\2\2\2\t\3\2\2\2\2\13\3\2\2\2\2\r\3\2\2\2\2\17\3\2\2\2"+
		"\2\21\3\2\2\2\2\23\3\2\2\2\2\25\3\2\2\2\2\27\3\2\2\2\2\31\3\2\2\2\2\33"+
		"\3\2\2\2\2#\3\2\2\2\2%\3\2\2\2\3\'\3\2\2\2\5)\3\2\2\2\7+\3\2\2\2\t-\3"+
		"\2\2\2\13/\3\2\2\2\r?\3\2\2\2\17A\3\2\2\2\21G\3\2\2\2\23V\3\2\2\2\25]"+
		"\3\2\2\2\27i\3\2\2\2\31k\3\2\2\2\33\u0080\3\2\2\2\35\u008a\3\2\2\2\37"+
		"\u008f\3\2\2\2!\u0092\3\2\2\2#\u0098\3\2\2\2%\u00a7\3\2\2\2\'(\7]\2\2"+
		"(\4\3\2\2\2)*\7_\2\2*\6\3\2\2\2+,\7*\2\2,\b\3\2\2\2-.\7+\2\2.\n\3\2\2"+
		"\2/\63\7$\2\2\60\62\n\2\2\2\61\60\3\2\2\2\62\65\3\2\2\2\63\61\3\2\2\2"+
		"\63\64\3\2\2\2\64\66\3\2\2\2\65\63\3\2\2\2\66\67\7$\2\2\67\f\3\2\2\28"+
		"9\7N\2\29:\7G\2\2:@\7P\2\2;<\7N\2\2<=\7G\2\2=>\7P\2\2>@\7\63\2\2?8\3\2"+
		"\2\2?;\3\2\2\2@\16\3\2\2\2AB\7C\2\2BC\7N\2\2CD\7K\2\2DE\7C\2\2EF\7U\2"+
		"\2F\20\3\2\2\2GH\7M\2\2HI\7G\2\2IJ\7[\2\2JK\7Y\2\2KL\7Q\2\2LM\7T\2\2M"+
		"N\7F\2\2NO\3\2\2\2OS\13\2\2\2PR\t\3\2\2QP\3\2\2\2RU\3\2\2\2SQ\3\2\2\2"+
		"ST\3\2\2\2T\22\3\2\2\2US\3\2\2\2VZ\t\4\2\2WY\t\5\2\2XW\3\2\2\2Y\\\3\2"+
		"\2\2ZX\3\2\2\2Z[\3\2\2\2[\24\3\2\2\2\\Z\3\2\2\2]a\7}\2\2^`\n\6\2\2_^\3"+
		"\2\2\2`c\3\2\2\2a_\3\2\2\2ab\3\2\2\2bd\3\2\2\2ca\3\2\2\2de\7\177\2\2e"+
		"\26\3\2\2\2fj\7@\2\2gh\7@\2\2hj\7?\2\2if\3\2\2\2ig\3\2\2\2j\30\3\2\2\2"+
		"kl\7~\2\2l\32\3\2\2\2mo\7/\2\2nm\3\2\2\2no\3\2\2\2op\3\2\2\2pq\5\35\17"+
		"\2qr\7\60\2\2rt\5\37\20\2su\5!\21\2ts\3\2\2\2tu\3\2\2\2u\u0081\3\2\2\2"+
		"vx\7/\2\2wv\3\2\2\2wx\3\2\2\2xy\3\2\2\2yz\5\35\17\2z{\5!\21\2{\u0081\3"+
		"\2\2\2|~\7/\2\2}|\3\2\2\2}~\3\2\2\2~\177\3\2\2\2\177\u0081\5\35\17\2\u0080"+
		"n\3\2\2\2\u0080w\3\2\2\2\u0080}\3\2\2\2\u0081\34\3\2\2\2\u0082\u008b\7"+
		"\62\2\2\u0083\u0087\t\7\2\2\u0084\u0086\t\3\2\2\u0085\u0084\3\2\2\2\u0086"+
		"\u0089\3\2\2\2\u0087\u0085\3\2\2\2\u0087\u0088\3\2\2\2\u0088\u008b\3\2"+
		"\2\2\u0089\u0087\3\2\2\2\u008a\u0082\3\2\2\2\u008a\u0083\3\2\2\2\u008b"+
		"\36\3\2\2\2\u008c\u008e\t\3\2\2\u008d\u008c\3\2\2\2\u008e\u0091\3\2\2"+
		"\2\u008f\u008d\3\2\2\2\u008f\u0090\3\2\2\2\u0090 \3\2\2\2\u0091\u008f"+
		"\3\2\2\2\u0092\u0094\t\b\2\2\u0093\u0095\t\t\2\2\u0094\u0093\3\2\2\2\u0094"+
		"\u0095\3\2\2\2\u0095\u0096\3\2\2\2\u0096\u0097\5\37\20\2\u0097\"\3\2\2"+
		"\2\u0098\u009c\7%\2\2\u0099\u009b\13\2\2\2\u009a\u0099\3\2\2\2\u009b\u009e"+
		"\3\2\2\2\u009c\u009d\3\2\2\2\u009c\u009a\3\2\2\2\u009d\u00a0\3\2\2\2\u009e"+
		"\u009c\3\2\2\2\u009f\u00a1\7\17\2\2\u00a0\u009f\3\2\2\2\u00a0\u00a1\3"+
		"\2\2\2\u00a1\u00a2\3\2\2\2\u00a2\u00a3\7\f\2\2\u00a3\u00a4\3\2\2\2\u00a4"+
		"\u00a5\b\22\2\2\u00a5$\3\2\2\2\u00a6\u00a8\t\n\2\2\u00a7\u00a6\3\2\2\2"+
		"\u00a8\u00a9\3\2\2\2\u00a9\u00a7\3\2\2\2\u00a9\u00aa\3\2\2\2\u00aa\u00ab"+
		"\3\2\2\2\u00ab\u00ac\b\23\3\2\u00ac&\3\2\2\2\25\2\63?SZaintw}\u0080\u0087"+
		"\u008a\u008f\u0094\u009c\u00a0\u00a9";
	public static final ATN _ATN =
		ATNSimulator.deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}
