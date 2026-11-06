
// Generated from dzshader_lexer.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  dzshader_lexer : public antlr4::Lexer {
public:
  enum {
    SHADER = 1, PROPERTIES = 2, SUBSHADER = 3, PASS = 4, SURFACE = 5, TAGS = 6,
    NAME = 7, MODEL = 8, ENTRY = 9, VARIANT = 10, CONSTANTS = 11, MODULES = 12,
    DEFINES = 13, REQUIRES = 14, VERTEX = 15, FRAGMENT = 16, TASK = 17,
    MESH = 18, COMPUTE = 19, CULL = 20, FRONT_FACE = 21, ZWRITE = 22, ZTEST = 23,
    DEPTH_BIAS = 24, BLEND = 25, BLEND_OP = 26, COLOR_MASK = 27, STENCIL = 28,
    SLANG_PROGRAM = 29, SLANG_INCLUDE = 30, TEXTURE_TYPE = 31, IDENTIFIER = 32,
    NUMBER = 33, STRING = 34, LBRACE = 35, RBRACE = 36, LPAREN = 37, RPAREN = 38,
    LBRACKET = 39, RBRACKET = 40, COMMA = 41, EQUAL = 42, SEMICOLON = 43,
    LINE_COMMENT = 44, BLOCK_COMMENT = 45, WHITESPACE = 46, PROGRAM_END = 47,
    PROGRAM_TEXT = 48
  };

  enum {
    PROGRAM = 1, RAW_STRING = 2
  };

  explicit dzshader_lexer(antlr4::CharStream *input);

  ~dzshader_lexer() override;


      bool isProgramEnd()
      {
          if (getCharPositionInLine() != 0)
          {
              return false;
          }
          auto previous = _input->LA(-2);
          if (previous == '\r')
          {
              previous = _input->LA(-3);
          }
          if (previous == '\\')
          {
              return false;
          }
          size_t offset = 1;
          while (_input->LA(offset) == ' ' || _input->LA(offset) == '\t')
          {
              ++offset;
          }
          for (char character : std::string("ENDSLANG"))
          {
              if (_input->LA(offset++) != size_t(character))
              {
                  return false;
              }
          }
          while (_input->LA(offset) == ' ' || _input->LA(offset) == '\t')
          {
              ++offset;
          }
          return _input->LA(offset) == '\r' || _input->LA(offset) == '\n' ||
                 _input->LA(offset) == antlr4::Token::EOF;
      }
      std::string rawDelimiter;

      bool isProgramSpecial()
      {
          const auto current = _input->LA(1);
          const auto next = _input->LA(2);
          return current == '"' || current == '\'' ||
                 (current == '/' && (next == '/' || next == '*')) || (current == 'R' && next == '"');
      }

      bool isRawStringEnd()
      {
          if (_input->LA(1) != ')')
          {
              return false;
          }
          size_t offset = 2;
          for (const auto character : rawDelimiter)
          {
              if (_input->LA(offset++) != size_t(character))
              {
                  return false;
              }
          }
          return _input->LA(offset) == '"';
      }


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  void action(antlr4::RuleContext *context, size_t ruleIndex, size_t actionIndex) override;

  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.
  void PROGRAM_RAW_STARTAction(antlr4::RuleContext *context, size_t actionIndex);

  // Individual semantic predicate functions triggered by sempred() above.
  bool PROGRAM_ENDSempred(antlr4::RuleContext *_localctx, size_t predicateIndex);
  bool PROGRAM_TEXTSempred(antlr4::RuleContext *_localctx, size_t predicateIndex);
  bool RAW_STRING_ENDSempred(antlr4::RuleContext *_localctx, size_t predicateIndex);
  bool RAW_STRING_TEXTSempred(antlr4::RuleContext *_localctx, size_t predicateIndex);

};
