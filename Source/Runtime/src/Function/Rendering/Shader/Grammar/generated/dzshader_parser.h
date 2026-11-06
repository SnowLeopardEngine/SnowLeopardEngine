
// Generated from dzshader_parser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  dzshader_parser : public antlr4::Parser {
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
    RuleFile = 0, RuleProperties = 1, RuleProperty = 2, RuleAttribute = 3,
    RuleAttributeArgument = 4, RulePropertyType = 5, RuleValue = 6, RuleVariant = 7,
    RuleConstants = 8, RuleModules = 9, RuleDefines = 10, RuleSubshader = 11,
    RuleTags = 12, RuleRequires = 13, RuleCommon = 14, RuleProgram = 15,
    RuleSurface = 16, RulePass = 17, RuleStage = 18, RuleState = 19, RuleStateValue = 20
  };

  explicit dzshader_parser(antlr4::TokenStream *input);

  dzshader_parser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~dzshader_parser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class FileContext;
  class PropertiesContext;
  class PropertyContext;
  class AttributeContext;
  class AttributeArgumentContext;
  class PropertyTypeContext;
  class ValueContext;
  class VariantContext;
  class ConstantsContext;
  class ModulesContext;
  class DefinesContext;
  class SubshaderContext;
  class TagsContext;
  class RequiresContext;
  class CommonContext;
  class ProgramContext;
  class SurfaceContext;
  class PassContext;
  class StageContext;
  class StateContext;
  class StateValueContext;

  class  FileContext : public antlr4::ParserRuleContext {
  public:
    FileContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *SHADER();
    antlr4::tree::TerminalNode *STRING();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    antlr4::tree::TerminalNode *EOF();
    std::vector<PropertiesContext *> properties();
    PropertiesContext* properties(size_t i);
    std::vector<VariantContext *> variant();
    VariantContext* variant(size_t i);
    std::vector<SubshaderContext *> subshader();
    SubshaderContext* subshader(size_t i);


  };

  FileContext* file();

  class  PropertiesContext : public antlr4::ParserRuleContext {
  public:
    PropertiesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PROPERTIES();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<PropertyContext *> property();
    PropertyContext* property(size_t i);


  };

  PropertiesContext* properties();

  class  PropertyContext : public antlr4::ParserRuleContext {
  public:
    PropertyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *STRING();
    antlr4::tree::TerminalNode *COMMA();
    PropertyTypeContext *propertyType();
    antlr4::tree::TerminalNode *RPAREN();
    antlr4::tree::TerminalNode *EQUAL();
    ValueContext *value();
    std::vector<AttributeContext *> attribute();
    AttributeContext* attribute(size_t i);


  };

  PropertyContext* property();

  class  AttributeContext : public antlr4::ParserRuleContext {
  public:
    AttributeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACKET();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *RBRACKET();
    antlr4::tree::TerminalNode *LPAREN();
    std::vector<AttributeArgumentContext *> attributeArgument();
    AttributeArgumentContext* attributeArgument(size_t i);
    antlr4::tree::TerminalNode *RPAREN();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);


  };

  AttributeContext* attribute();

  class  AttributeArgumentContext : public antlr4::ParserRuleContext {
  public:
    AttributeArgumentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *NUMBER();
    antlr4::tree::TerminalNode *STRING();


  };

  AttributeArgumentContext* attributeArgument();

  class  PropertyTypeContext : public antlr4::ParserRuleContext {
  public:
    PropertyTypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *TEXTURE_TYPE();
    antlr4::tree::TerminalNode *LPAREN();
    std::vector<antlr4::tree::TerminalNode *> NUMBER();
    antlr4::tree::TerminalNode* NUMBER(size_t i);
    antlr4::tree::TerminalNode *COMMA();
    antlr4::tree::TerminalNode *RPAREN();


  };

  PropertyTypeContext* propertyType();

  class  ValueContext : public antlr4::ParserRuleContext {
  public:
    ValueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> NUMBER();
    antlr4::tree::TerminalNode* NUMBER(size_t i);
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *STRING();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);


  };

  ValueContext* value();

  class  VariantContext : public antlr4::ParserRuleContext {
  public:
    VariantContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *VARIANT();
    antlr4::tree::TerminalNode *STRING();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<ConstantsContext *> constants();
    ConstantsContext* constants(size_t i);
    std::vector<ModulesContext *> modules();
    ModulesContext* modules(size_t i);
    std::vector<DefinesContext *> defines();
    DefinesContext* defines(size_t i);


  };

  VariantContext* variant();

  class  ConstantsContext : public antlr4::ParserRuleContext {
  public:
    ConstantsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CONSTANTS();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<antlr4::tree::TerminalNode *> IDENTIFIER();
    antlr4::tree::TerminalNode* IDENTIFIER(size_t i);
    std::vector<antlr4::tree::TerminalNode *> EQUAL();
    antlr4::tree::TerminalNode* EQUAL(size_t i);
    std::vector<antlr4::tree::TerminalNode *> NUMBER();
    antlr4::tree::TerminalNode* NUMBER(size_t i);


  };

  ConstantsContext* constants();

  class  ModulesContext : public antlr4::ParserRuleContext {
  public:
    ModulesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *MODULES();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<antlr4::tree::TerminalNode *> STRING();
    antlr4::tree::TerminalNode* STRING(size_t i);


  };

  ModulesContext* modules();

  class  DefinesContext : public antlr4::ParserRuleContext {
  public:
    DefinesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DEFINES();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<antlr4::tree::TerminalNode *> IDENTIFIER();
    antlr4::tree::TerminalNode* IDENTIFIER(size_t i);
    std::vector<antlr4::tree::TerminalNode *> EQUAL();
    antlr4::tree::TerminalNode* EQUAL(size_t i);
    std::vector<antlr4::tree::TerminalNode *> NUMBER();
    antlr4::tree::TerminalNode* NUMBER(size_t i);
    std::vector<antlr4::tree::TerminalNode *> STRING();
    antlr4::tree::TerminalNode* STRING(size_t i);


  };

  DefinesContext* defines();

  class  SubshaderContext : public antlr4::ParserRuleContext {
  public:
    SubshaderContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *SUBSHADER();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<TagsContext *> tags();
    TagsContext* tags(size_t i);
    std::vector<RequiresContext *> requires_();
    RequiresContext* requires_(size_t i);
    std::vector<StateContext *> state();
    StateContext* state(size_t i);
    std::vector<CommonContext *> common();
    CommonContext* common(size_t i);
    std::vector<SurfaceContext *> surface();
    SurfaceContext* surface(size_t i);
    std::vector<PassContext *> pass();
    PassContext* pass(size_t i);


  };

  SubshaderContext* subshader();

  class  TagsContext : public antlr4::ParserRuleContext {
  public:
    TagsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TAGS();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<antlr4::tree::TerminalNode *> STRING();
    antlr4::tree::TerminalNode* STRING(size_t i);
    std::vector<antlr4::tree::TerminalNode *> EQUAL();
    antlr4::tree::TerminalNode* EQUAL(size_t i);


  };

  TagsContext* tags();

  class  RequiresContext : public antlr4::ParserRuleContext {
  public:
    RequiresContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *REQUIRES();
    std::vector<antlr4::tree::TerminalNode *> IDENTIFIER();
    antlr4::tree::TerminalNode* IDENTIFIER(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);


  };

  RequiresContext* requires_();

  class  CommonContext : public antlr4::ParserRuleContext {
  public:
    CommonContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *SLANG_INCLUDE();
    antlr4::tree::TerminalNode *PROGRAM_END();
    std::vector<antlr4::tree::TerminalNode *> PROGRAM_TEXT();
    antlr4::tree::TerminalNode* PROGRAM_TEXT(size_t i);


  };

  CommonContext* common();

  class  ProgramContext : public antlr4::ParserRuleContext {
  public:
    ProgramContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *SLANG_PROGRAM();
    antlr4::tree::TerminalNode *PROGRAM_END();
    std::vector<antlr4::tree::TerminalNode *> PROGRAM_TEXT();
    antlr4::tree::TerminalNode* PROGRAM_TEXT(size_t i);


  };

  ProgramContext* program();

  class  SurfaceContext : public antlr4::ParserRuleContext {
  public:
    SurfaceContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *SURFACE();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<antlr4::tree::TerminalNode *> MODEL();
    antlr4::tree::TerminalNode* MODEL(size_t i);
    std::vector<antlr4::tree::TerminalNode *> IDENTIFIER();
    antlr4::tree::TerminalNode* IDENTIFIER(size_t i);
    std::vector<antlr4::tree::TerminalNode *> ENTRY();
    antlr4::tree::TerminalNode* ENTRY(size_t i);
    std::vector<StateContext *> state();
    StateContext* state(size_t i);
    std::vector<ProgramContext *> program();
    ProgramContext* program(size_t i);


  };

  SurfaceContext* surface();

  class  PassContext : public antlr4::ParserRuleContext {
  public:
    PassContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PASS();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<antlr4::tree::TerminalNode *> NAME();
    antlr4::tree::TerminalNode* NAME(size_t i);
    std::vector<antlr4::tree::TerminalNode *> STRING();
    antlr4::tree::TerminalNode* STRING(size_t i);
    std::vector<TagsContext *> tags();
    TagsContext* tags(size_t i);
    std::vector<StageContext *> stage();
    StageContext* stage(size_t i);
    std::vector<RequiresContext *> requires_();
    RequiresContext* requires_(size_t i);
    std::vector<StateContext *> state();
    StateContext* state(size_t i);
    std::vector<ProgramContext *> program();
    ProgramContext* program(size_t i);


  };

  PassContext* pass();

  class  StageContext : public antlr4::ParserRuleContext {
  public:
    StageContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *VERTEX();
    antlr4::tree::TerminalNode *FRAGMENT();
    antlr4::tree::TerminalNode *TASK();
    antlr4::tree::TerminalNode *MESH();
    antlr4::tree::TerminalNode *COMPUTE();


  };

  StageContext* stage();

  class  StateContext : public antlr4::ParserRuleContext {
  public:
    StateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CULL();
    std::vector<StateValueContext *> stateValue();
    StateValueContext* stateValue(size_t i);
    antlr4::tree::TerminalNode *FRONT_FACE();
    antlr4::tree::TerminalNode *ZWRITE();
    antlr4::tree::TerminalNode *ZTEST();
    antlr4::tree::TerminalNode *DEPTH_BIAS();
    antlr4::tree::TerminalNode *BLEND();
    antlr4::tree::TerminalNode *COMMA();
    antlr4::tree::TerminalNode *BLEND_OP();
    antlr4::tree::TerminalNode *COLOR_MASK();
    antlr4::tree::TerminalNode *NUMBER();
    antlr4::tree::TerminalNode *STENCIL();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<antlr4::tree::TerminalNode *> IDENTIFIER();
    antlr4::tree::TerminalNode* IDENTIFIER(size_t i);


  };

  StateContext* state();

  class  StateValueContext : public antlr4::ParserRuleContext {
  public:
    StateValueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *NUMBER();
    antlr4::tree::TerminalNode *LBRACKET();
    antlr4::tree::TerminalNode *RBRACKET();


  };

  StateValueContext* stateValue();


  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};
