
// Generated from dzshader_parser.g4 by ANTLR 4.13.2



#include "dzshader_parser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct Vshader_parserStaticData final {
  Vshader_parserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  Vshader_parserStaticData(const Vshader_parserStaticData&) = delete;
  Vshader_parserStaticData(Vshader_parserStaticData&&) = delete;
  Vshader_parserStaticData& operator=(const Vshader_parserStaticData&) = delete;
  Vshader_parserStaticData& operator=(Vshader_parserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag dzshader_parserParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<Vshader_parserStaticData> dzshader_parserParserStaticData = nullptr;

void dzshader_parserParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (dzshader_parserParserStaticData != nullptr) {
    return;
  }
#else
  assert(dzshader_parserParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<Vshader_parserStaticData>(
    std::vector<std::string>{
      "file", "properties", "property", "attribute", "attributeArgument",
      "propertyType", "value", "variant", "constants", "modules", "defines",
      "subshader", "tags", "requires", "common", "program", "surface", "pass",
      "stage", "state", "stateValue"
    },
    std::vector<std::string>{
      "", "'Shader'", "'Properties'", "'SubShader'", "'Pass'", "'Surface'",
      "'Tags'", "'Name'", "'Model'", "'Entry'", "'Variant'", "'Constants'",
      "'Modules'", "'Defines'", "'Requires'", "'Vertex'", "'Fragment'",
      "'Task'", "'Mesh'", "'Compute'", "'Cull'", "'FrontFace'", "'ZWrite'",
      "'ZTest'", "'DepthBias'", "'Blend'", "'BlendOp'", "'ColorMask'", "'Stencil'",
      "", "", "", "", "", "", "'{'", "'}'", "'('", "')'", "'['", "']'",
      "','", "'='", "';'"
    },
    std::vector<std::string>{
      "", "SHADER", "PROPERTIES", "SUBSHADER", "PASS", "SURFACE", "TAGS",
      "NAME", "MODEL", "ENTRY", "VARIANT", "CONSTANTS", "MODULES", "DEFINES",
      "REQUIRES", "VERTEX", "FRAGMENT", "TASK", "MESH", "COMPUTE", "CULL",
      "FRONT_FACE", "ZWRITE", "ZTEST", "DEPTH_BIAS", "BLEND", "BLEND_OP",
      "COLOR_MASK", "STENCIL", "SLANG_PROGRAM", "SLANG_INCLUDE", "TEXTURE_TYPE",
      "IDENTIFIER", "NUMBER", "STRING", "LBRACE", "RBRACE", "LPAREN", "RPAREN",
      "LBRACKET", "RBRACKET", "COMMA", "EQUAL", "SEMICOLON", "LINE_COMMENT",
      "BLOCK_COMMENT", "WHITESPACE", "PROGRAM_END", "PROGRAM_TEXT"
    }
  );
  static const int32_t serializedATNSegment[] = {
      4,1,48,319,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
      7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
      14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,1,0,1,
      0,1,0,1,0,1,0,1,0,5,0,49,8,0,10,0,12,0,52,9,0,1,0,1,0,1,0,1,1,1,1,1,1,
      5,1,60,8,1,10,1,12,1,63,9,1,1,1,1,1,1,2,5,2,68,8,2,10,2,12,2,71,9,2,1,
      2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,3,1,3,1,3,1,3,1,3,1,3,5,3,88,8,3,
      10,3,12,3,91,9,3,1,3,1,3,3,3,95,8,3,1,3,1,3,1,4,1,4,1,5,1,5,1,5,1,5,1,
      5,1,5,3,5,107,8,5,1,6,1,6,1,6,1,6,1,6,3,6,114,8,6,1,6,1,6,1,6,1,6,5,6,
      120,8,6,10,6,12,6,123,9,6,1,6,3,6,126,8,6,1,7,1,7,1,7,1,7,1,7,1,7,5,7,
      134,8,7,10,7,12,7,137,9,7,1,7,1,7,1,8,1,8,1,8,1,8,1,8,1,8,5,8,147,8,8,
      10,8,12,8,150,9,8,1,8,1,8,1,9,1,9,1,9,5,9,157,8,9,10,9,12,9,160,9,9,1,
      9,1,9,1,10,1,10,1,10,1,10,1,10,5,10,169,8,10,10,10,12,10,172,9,10,1,10,
      1,10,1,11,1,11,1,11,1,11,1,11,1,11,1,11,1,11,5,11,184,8,11,10,11,12,11,
      187,9,11,1,11,1,11,1,12,1,12,1,12,1,12,1,12,5,12,196,8,12,10,12,12,12,
      199,9,12,1,12,1,12,1,13,1,13,1,13,1,13,5,13,207,8,13,10,13,12,13,210,
      9,13,1,14,1,14,5,14,214,8,14,10,14,12,14,217,9,14,1,14,1,14,1,15,1,15,
      5,15,223,8,15,10,15,12,15,226,9,15,1,15,1,15,1,16,1,16,1,16,1,16,1,16,
      1,16,1,16,1,16,5,16,238,8,16,10,16,12,16,241,9,16,1,16,1,16,1,17,1,17,
      1,17,1,17,1,17,1,17,1,17,1,17,1,17,5,17,254,8,17,10,17,12,17,257,9,17,
      1,17,1,17,1,18,1,18,1,18,1,19,1,19,1,19,1,19,1,19,1,19,1,19,1,19,1,19,
      1,19,1,19,1,19,3,19,276,8,19,1,19,1,19,1,19,1,19,1,19,1,19,1,19,3,19,
      285,8,19,3,19,287,8,19,1,19,1,19,1,19,1,19,3,19,293,8,19,1,19,1,19,1,
      19,3,19,298,8,19,1,19,1,19,1,19,1,19,5,19,304,8,19,10,19,12,19,307,9,
      19,1,19,3,19,310,8,19,1,20,1,20,1,20,1,20,1,20,3,20,317,8,20,1,20,0,0,
      21,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,0,4,1,0,
      32,34,1,0,31,32,1,0,32,33,1,0,15,19,352,0,42,1,0,0,0,2,56,1,0,0,0,4,69,
      1,0,0,0,6,81,1,0,0,0,8,98,1,0,0,0,10,100,1,0,0,0,12,125,1,0,0,0,14,127,
      1,0,0,0,16,140,1,0,0,0,18,153,1,0,0,0,20,163,1,0,0,0,22,175,1,0,0,0,24,
      190,1,0,0,0,26,202,1,0,0,0,28,211,1,0,0,0,30,220,1,0,0,0,32,229,1,0,0,
      0,34,244,1,0,0,0,36,260,1,0,0,0,38,309,1,0,0,0,40,316,1,0,0,0,42,43,5,
      1,0,0,43,44,5,34,0,0,44,50,5,35,0,0,45,49,3,2,1,0,46,49,3,14,7,0,47,49,
      3,22,11,0,48,45,1,0,0,0,48,46,1,0,0,0,48,47,1,0,0,0,49,52,1,0,0,0,50,
      48,1,0,0,0,50,51,1,0,0,0,51,53,1,0,0,0,52,50,1,0,0,0,53,54,5,36,0,0,54,
      55,5,0,0,1,55,1,1,0,0,0,56,57,5,2,0,0,57,61,5,35,0,0,58,60,3,4,2,0,59,
      58,1,0,0,0,60,63,1,0,0,0,61,59,1,0,0,0,61,62,1,0,0,0,62,64,1,0,0,0,63,
      61,1,0,0,0,64,65,5,36,0,0,65,3,1,0,0,0,66,68,3,6,3,0,67,66,1,0,0,0,68,
      71,1,0,0,0,69,67,1,0,0,0,69,70,1,0,0,0,70,72,1,0,0,0,71,69,1,0,0,0,72,
      73,5,32,0,0,73,74,5,37,0,0,74,75,5,34,0,0,75,76,5,41,0,0,76,77,3,10,5,
      0,77,78,5,38,0,0,78,79,5,42,0,0,79,80,3,12,6,0,80,5,1,0,0,0,81,82,5,39,
      0,0,82,94,5,32,0,0,83,84,5,37,0,0,84,89,3,8,4,0,85,86,5,41,0,0,86,88,
      3,8,4,0,87,85,1,0,0,0,88,91,1,0,0,0,89,87,1,0,0,0,89,90,1,0,0,0,90,92,
      1,0,0,0,91,89,1,0,0,0,92,93,5,38,0,0,93,95,1,0,0,0,94,83,1,0,0,0,94,95,
      1,0,0,0,95,96,1,0,0,0,96,97,5,40,0,0,97,7,1,0,0,0,98,99,7,0,0,0,99,9,
      1,0,0,0,100,106,7,1,0,0,101,102,5,37,0,0,102,103,5,33,0,0,103,104,5,41,
      0,0,104,105,5,33,0,0,105,107,5,38,0,0,106,101,1,0,0,0,106,107,1,0,0,0,
      107,11,1,0,0,0,108,126,5,33,0,0,109,126,5,32,0,0,110,113,5,34,0,0,111,
      112,5,35,0,0,112,114,5,36,0,0,113,111,1,0,0,0,113,114,1,0,0,0,114,126,
      1,0,0,0,115,116,5,37,0,0,116,121,5,33,0,0,117,118,5,41,0,0,118,120,5,
      33,0,0,119,117,1,0,0,0,120,123,1,0,0,0,121,119,1,0,0,0,121,122,1,0,0,
      0,122,124,1,0,0,0,123,121,1,0,0,0,124,126,5,38,0,0,125,108,1,0,0,0,125,
      109,1,0,0,0,125,110,1,0,0,0,125,115,1,0,0,0,126,13,1,0,0,0,127,128,5,
      10,0,0,128,129,5,34,0,0,129,135,5,35,0,0,130,134,3,16,8,0,131,134,3,18,
      9,0,132,134,3,20,10,0,133,130,1,0,0,0,133,131,1,0,0,0,133,132,1,0,0,0,
      134,137,1,0,0,0,135,133,1,0,0,0,135,136,1,0,0,0,136,138,1,0,0,0,137,135,
      1,0,0,0,138,139,5,36,0,0,139,15,1,0,0,0,140,141,5,11,0,0,141,148,5,35,
      0,0,142,143,5,32,0,0,143,144,5,32,0,0,144,145,5,42,0,0,145,147,7,2,0,
      0,146,142,1,0,0,0,147,150,1,0,0,0,148,146,1,0,0,0,148,149,1,0,0,0,149,
      151,1,0,0,0,150,148,1,0,0,0,151,152,5,36,0,0,152,17,1,0,0,0,153,154,5,
      12,0,0,154,158,5,35,0,0,155,157,5,34,0,0,156,155,1,0,0,0,157,160,1,0,
      0,0,158,156,1,0,0,0,158,159,1,0,0,0,159,161,1,0,0,0,160,158,1,0,0,0,161,
      162,5,36,0,0,162,19,1,0,0,0,163,164,5,13,0,0,164,170,5,35,0,0,165,166,
      5,32,0,0,166,167,5,42,0,0,167,169,7,0,0,0,168,165,1,0,0,0,169,172,1,0,
      0,0,170,168,1,0,0,0,170,171,1,0,0,0,171,173,1,0,0,0,172,170,1,0,0,0,173,
      174,5,36,0,0,174,21,1,0,0,0,175,176,5,3,0,0,176,185,5,35,0,0,177,184,
      3,24,12,0,178,184,3,26,13,0,179,184,3,38,19,0,180,184,3,28,14,0,181,184,
      3,32,16,0,182,184,3,34,17,0,183,177,1,0,0,0,183,178,1,0,0,0,183,179,1,
      0,0,0,183,180,1,0,0,0,183,181,1,0,0,0,183,182,1,0,0,0,184,187,1,0,0,0,
      185,183,1,0,0,0,185,186,1,0,0,0,186,188,1,0,0,0,187,185,1,0,0,0,188,189,
      5,36,0,0,189,23,1,0,0,0,190,191,5,6,0,0,191,197,5,35,0,0,192,193,5,34,
      0,0,193,194,5,42,0,0,194,196,5,34,0,0,195,192,1,0,0,0,196,199,1,0,0,0,
      197,195,1,0,0,0,197,198,1,0,0,0,198,200,1,0,0,0,199,197,1,0,0,0,200,201,
      5,36,0,0,201,25,1,0,0,0,202,203,5,14,0,0,203,208,5,32,0,0,204,205,5,41,
      0,0,205,207,5,32,0,0,206,204,1,0,0,0,207,210,1,0,0,0,208,206,1,0,0,0,
      208,209,1,0,0,0,209,27,1,0,0,0,210,208,1,0,0,0,211,215,5,30,0,0,212,214,
      5,48,0,0,213,212,1,0,0,0,214,217,1,0,0,0,215,213,1,0,0,0,215,216,1,0,
      0,0,216,218,1,0,0,0,217,215,1,0,0,0,218,219,5,47,0,0,219,29,1,0,0,0,220,
      224,5,29,0,0,221,223,5,48,0,0,222,221,1,0,0,0,223,226,1,0,0,0,224,222,
      1,0,0,0,224,225,1,0,0,0,225,227,1,0,0,0,226,224,1,0,0,0,227,228,5,47,
      0,0,228,31,1,0,0,0,229,230,5,5,0,0,230,239,5,35,0,0,231,232,5,8,0,0,232,
      238,5,32,0,0,233,234,5,9,0,0,234,238,5,32,0,0,235,238,3,38,19,0,236,238,
      3,30,15,0,237,231,1,0,0,0,237,233,1,0,0,0,237,235,1,0,0,0,237,236,1,0,
      0,0,238,241,1,0,0,0,239,237,1,0,0,0,239,240,1,0,0,0,240,242,1,0,0,0,241,
      239,1,0,0,0,242,243,5,36,0,0,243,33,1,0,0,0,244,245,5,4,0,0,245,255,5,
      35,0,0,246,247,5,7,0,0,247,254,5,34,0,0,248,254,3,24,12,0,249,254,3,36,
      18,0,250,254,3,26,13,0,251,254,3,38,19,0,252,254,3,30,15,0,253,246,1,
      0,0,0,253,248,1,0,0,0,253,249,1,0,0,0,253,250,1,0,0,0,253,251,1,0,0,0,
      253,252,1,0,0,0,254,257,1,0,0,0,255,253,1,0,0,0,255,256,1,0,0,0,256,258,
      1,0,0,0,257,255,1,0,0,0,258,259,5,36,0,0,259,35,1,0,0,0,260,261,7,3,0,
      0,261,262,5,32,0,0,262,37,1,0,0,0,263,264,5,20,0,0,264,310,3,40,20,0,
      265,266,5,21,0,0,266,310,3,40,20,0,267,268,5,22,0,0,268,310,3,40,20,0,
      269,270,5,23,0,0,270,310,3,40,20,0,271,272,5,24,0,0,272,273,3,40,20,0,
      273,275,3,40,20,0,274,276,3,40,20,0,275,274,1,0,0,0,275,276,1,0,0,0,276,
      310,1,0,0,0,277,278,5,25,0,0,278,286,3,40,20,0,279,284,3,40,20,0,280,
      281,5,41,0,0,281,282,3,40,20,0,282,283,3,40,20,0,283,285,1,0,0,0,284,
      280,1,0,0,0,284,285,1,0,0,0,285,287,1,0,0,0,286,279,1,0,0,0,286,287,1,
      0,0,0,287,310,1,0,0,0,288,289,5,26,0,0,289,292,3,40,20,0,290,291,5,41,
      0,0,291,293,3,40,20,0,292,290,1,0,0,0,292,293,1,0,0,0,293,310,1,0,0,0,
      294,295,5,27,0,0,295,297,3,40,20,0,296,298,5,33,0,0,297,296,1,0,0,0,297,
      298,1,0,0,0,298,310,1,0,0,0,299,300,5,28,0,0,300,305,5,35,0,0,301,302,
      5,32,0,0,302,304,3,40,20,0,303,301,1,0,0,0,304,307,1,0,0,0,305,303,1,
      0,0,0,305,306,1,0,0,0,306,308,1,0,0,0,307,305,1,0,0,0,308,310,5,36,0,
      0,309,263,1,0,0,0,309,265,1,0,0,0,309,267,1,0,0,0,309,269,1,0,0,0,309,
      271,1,0,0,0,309,277,1,0,0,0,309,288,1,0,0,0,309,294,1,0,0,0,309,299,1,
      0,0,0,310,39,1,0,0,0,311,317,5,32,0,0,312,317,5,33,0,0,313,314,5,39,0,
      0,314,315,5,32,0,0,315,317,5,40,0,0,316,311,1,0,0,0,316,312,1,0,0,0,316,
      313,1,0,0,0,317,41,1,0,0,0,33,48,50,61,69,89,94,106,113,121,125,133,135,
      148,158,170,183,185,197,208,215,224,237,239,253,255,275,284,286,292,297,
      305,309,316
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) {
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  dzshader_parserParserStaticData = std::move(staticData);
}

}

dzshader_parser::dzshader_parser(TokenStream *input) : dzshader_parser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

dzshader_parser::dzshader_parser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  dzshader_parser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *dzshader_parserParserStaticData->atn, dzshader_parserParserStaticData->decisionToDFA, dzshader_parserParserStaticData->sharedContextCache, options);
}

dzshader_parser::~dzshader_parser() {
  delete _interpreter;
}

const atn::ATN& dzshader_parser::getATN() const {
  return *dzshader_parserParserStaticData->atn;
}

std::string dzshader_parser::getGrammarFileName() const {
  return "dzshader_parser.g4";
}

const std::vector<std::string>& dzshader_parser::getRuleNames() const {
  return dzshader_parserParserStaticData->ruleNames;
}

const dfa::Vocabulary& dzshader_parser::getVocabulary() const {
  return dzshader_parserParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView dzshader_parser::getSerializedATN() const {
  return dzshader_parserParserStaticData->serializedATN;
}


//----------------- FileContext ------------------------------------------------------------------

dzshader_parser::FileContext::FileContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::FileContext::SHADER() {
  return getToken(dzshader_parser::SHADER, 0);
}

tree::TerminalNode* dzshader_parser::FileContext::STRING() {
  return getToken(dzshader_parser::STRING, 0);
}

tree::TerminalNode* dzshader_parser::FileContext::LBRACE() {
  return getToken(dzshader_parser::LBRACE, 0);
}

tree::TerminalNode* dzshader_parser::FileContext::RBRACE() {
  return getToken(dzshader_parser::RBRACE, 0);
}

tree::TerminalNode* dzshader_parser::FileContext::EOF() {
  return getToken(dzshader_parser::EOF, 0);
}

std::vector<dzshader_parser::PropertiesContext *> dzshader_parser::FileContext::properties() {
  return getRuleContexts<dzshader_parser::PropertiesContext>();
}

dzshader_parser::PropertiesContext* dzshader_parser::FileContext::properties(size_t i) {
  return getRuleContext<dzshader_parser::PropertiesContext>(i);
}

std::vector<dzshader_parser::VariantContext *> dzshader_parser::FileContext::variant() {
  return getRuleContexts<dzshader_parser::VariantContext>();
}

dzshader_parser::VariantContext* dzshader_parser::FileContext::variant(size_t i) {
  return getRuleContext<dzshader_parser::VariantContext>(i);
}

std::vector<dzshader_parser::SubshaderContext *> dzshader_parser::FileContext::subshader() {
  return getRuleContexts<dzshader_parser::SubshaderContext>();
}

dzshader_parser::SubshaderContext* dzshader_parser::FileContext::subshader(size_t i) {
  return getRuleContext<dzshader_parser::SubshaderContext>(i);
}


size_t dzshader_parser::FileContext::getRuleIndex() const {
  return dzshader_parser::RuleFile;
}


dzshader_parser::FileContext* dzshader_parser::file() {
  FileContext *_localctx = _tracker.createInstance<FileContext>(_ctx, getState());
  enterRule(_localctx, 0, dzshader_parser::RuleFile);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(42);
    match(dzshader_parser::SHADER);
    setState(43);
    match(dzshader_parser::STRING);
    setState(44);
    match(dzshader_parser::LBRACE);
    setState(50);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1036) != 0)) {
      setState(48);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case dzshader_parser::PROPERTIES: {
          setState(45);
          properties();
          break;
        }

        case dzshader_parser::VARIANT: {
          setState(46);
          variant();
          break;
        }

        case dzshader_parser::SUBSHADER: {
          setState(47);
          subshader();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(52);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(53);
    match(dzshader_parser::RBRACE);
    setState(54);
    match(dzshader_parser::EOF);

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PropertiesContext ------------------------------------------------------------------

dzshader_parser::PropertiesContext::PropertiesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::PropertiesContext::PROPERTIES() {
  return getToken(dzshader_parser::PROPERTIES, 0);
}

tree::TerminalNode* dzshader_parser::PropertiesContext::LBRACE() {
  return getToken(dzshader_parser::LBRACE, 0);
}

tree::TerminalNode* dzshader_parser::PropertiesContext::RBRACE() {
  return getToken(dzshader_parser::RBRACE, 0);
}

std::vector<dzshader_parser::PropertyContext *> dzshader_parser::PropertiesContext::property() {
  return getRuleContexts<dzshader_parser::PropertyContext>();
}

dzshader_parser::PropertyContext* dzshader_parser::PropertiesContext::property(size_t i) {
  return getRuleContext<dzshader_parser::PropertyContext>(i);
}


size_t dzshader_parser::PropertiesContext::getRuleIndex() const {
  return dzshader_parser::RuleProperties;
}


dzshader_parser::PropertiesContext* dzshader_parser::properties() {
  PropertiesContext *_localctx = _tracker.createInstance<PropertiesContext>(_ctx, getState());
  enterRule(_localctx, 2, dzshader_parser::RuleProperties);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(56);
    match(dzshader_parser::PROPERTIES);
    setState(57);
    match(dzshader_parser::LBRACE);
    setState(61);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == dzshader_parser::IDENTIFIER

    || _la == dzshader_parser::LBRACKET) {
      setState(58);
      property();
      setState(63);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(64);
    match(dzshader_parser::RBRACE);

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PropertyContext ------------------------------------------------------------------

dzshader_parser::PropertyContext::PropertyContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::PropertyContext::IDENTIFIER() {
  return getToken(dzshader_parser::IDENTIFIER, 0);
}

tree::TerminalNode* dzshader_parser::PropertyContext::LPAREN() {
  return getToken(dzshader_parser::LPAREN, 0);
}

tree::TerminalNode* dzshader_parser::PropertyContext::STRING() {
  return getToken(dzshader_parser::STRING, 0);
}

tree::TerminalNode* dzshader_parser::PropertyContext::COMMA() {
  return getToken(dzshader_parser::COMMA, 0);
}

dzshader_parser::PropertyTypeContext* dzshader_parser::PropertyContext::propertyType() {
  return getRuleContext<dzshader_parser::PropertyTypeContext>(0);
}

tree::TerminalNode* dzshader_parser::PropertyContext::RPAREN() {
  return getToken(dzshader_parser::RPAREN, 0);
}

tree::TerminalNode* dzshader_parser::PropertyContext::EQUAL() {
  return getToken(dzshader_parser::EQUAL, 0);
}

dzshader_parser::ValueContext* dzshader_parser::PropertyContext::value() {
  return getRuleContext<dzshader_parser::ValueContext>(0);
}

std::vector<dzshader_parser::AttributeContext *> dzshader_parser::PropertyContext::attribute() {
  return getRuleContexts<dzshader_parser::AttributeContext>();
}

dzshader_parser::AttributeContext* dzshader_parser::PropertyContext::attribute(size_t i) {
  return getRuleContext<dzshader_parser::AttributeContext>(i);
}


size_t dzshader_parser::PropertyContext::getRuleIndex() const {
  return dzshader_parser::RuleProperty;
}


dzshader_parser::PropertyContext* dzshader_parser::property() {
  PropertyContext *_localctx = _tracker.createInstance<PropertyContext>(_ctx, getState());
  enterRule(_localctx, 4, dzshader_parser::RuleProperty);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(69);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == dzshader_parser::LBRACKET) {
      setState(66);
      attribute();
      setState(71);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(72);
    match(dzshader_parser::IDENTIFIER);
    setState(73);
    match(dzshader_parser::LPAREN);
    setState(74);
    match(dzshader_parser::STRING);
    setState(75);
    match(dzshader_parser::COMMA);
    setState(76);
    propertyType();
    setState(77);
    match(dzshader_parser::RPAREN);
    setState(78);
    match(dzshader_parser::EQUAL);
    setState(79);
    value();

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttributeContext ------------------------------------------------------------------

dzshader_parser::AttributeContext::AttributeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::AttributeContext::LBRACKET() {
  return getToken(dzshader_parser::LBRACKET, 0);
}

tree::TerminalNode* dzshader_parser::AttributeContext::IDENTIFIER() {
  return getToken(dzshader_parser::IDENTIFIER, 0);
}

tree::TerminalNode* dzshader_parser::AttributeContext::RBRACKET() {
  return getToken(dzshader_parser::RBRACKET, 0);
}

tree::TerminalNode* dzshader_parser::AttributeContext::LPAREN() {
  return getToken(dzshader_parser::LPAREN, 0);
}

std::vector<dzshader_parser::AttributeArgumentContext *> dzshader_parser::AttributeContext::attributeArgument() {
  return getRuleContexts<dzshader_parser::AttributeArgumentContext>();
}

dzshader_parser::AttributeArgumentContext* dzshader_parser::AttributeContext::attributeArgument(size_t i) {
  return getRuleContext<dzshader_parser::AttributeArgumentContext>(i);
}

tree::TerminalNode* dzshader_parser::AttributeContext::RPAREN() {
  return getToken(dzshader_parser::RPAREN, 0);
}

std::vector<tree::TerminalNode *> dzshader_parser::AttributeContext::COMMA() {
  return getTokens(dzshader_parser::COMMA);
}

tree::TerminalNode* dzshader_parser::AttributeContext::COMMA(size_t i) {
  return getToken(dzshader_parser::COMMA, i);
}


size_t dzshader_parser::AttributeContext::getRuleIndex() const {
  return dzshader_parser::RuleAttribute;
}


dzshader_parser::AttributeContext* dzshader_parser::attribute() {
  AttributeContext *_localctx = _tracker.createInstance<AttributeContext>(_ctx, getState());
  enterRule(_localctx, 6, dzshader_parser::RuleAttribute);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(81);
    match(dzshader_parser::LBRACKET);
    setState(82);
    match(dzshader_parser::IDENTIFIER);
    setState(94);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == dzshader_parser::LPAREN) {
      setState(83);
      match(dzshader_parser::LPAREN);
      setState(84);
      attributeArgument();
      setState(89);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == dzshader_parser::COMMA) {
        setState(85);
        match(dzshader_parser::COMMA);
        setState(86);
        attributeArgument();
        setState(91);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(92);
      match(dzshader_parser::RPAREN);
    }
    setState(96);
    match(dzshader_parser::RBRACKET);

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttributeArgumentContext ------------------------------------------------------------------

dzshader_parser::AttributeArgumentContext::AttributeArgumentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::AttributeArgumentContext::IDENTIFIER() {
  return getToken(dzshader_parser::IDENTIFIER, 0);
}

tree::TerminalNode* dzshader_parser::AttributeArgumentContext::NUMBER() {
  return getToken(dzshader_parser::NUMBER, 0);
}

tree::TerminalNode* dzshader_parser::AttributeArgumentContext::STRING() {
  return getToken(dzshader_parser::STRING, 0);
}


size_t dzshader_parser::AttributeArgumentContext::getRuleIndex() const {
  return dzshader_parser::RuleAttributeArgument;
}


dzshader_parser::AttributeArgumentContext* dzshader_parser::attributeArgument() {
  AttributeArgumentContext *_localctx = _tracker.createInstance<AttributeArgumentContext>(_ctx, getState());
  enterRule(_localctx, 8, dzshader_parser::RuleAttributeArgument);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(98);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 30064771072) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PropertyTypeContext ------------------------------------------------------------------

dzshader_parser::PropertyTypeContext::PropertyTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::PropertyTypeContext::IDENTIFIER() {
  return getToken(dzshader_parser::IDENTIFIER, 0);
}

tree::TerminalNode* dzshader_parser::PropertyTypeContext::TEXTURE_TYPE() {
  return getToken(dzshader_parser::TEXTURE_TYPE, 0);
}

tree::TerminalNode* dzshader_parser::PropertyTypeContext::LPAREN() {
  return getToken(dzshader_parser::LPAREN, 0);
}

std::vector<tree::TerminalNode *> dzshader_parser::PropertyTypeContext::NUMBER() {
  return getTokens(dzshader_parser::NUMBER);
}

tree::TerminalNode* dzshader_parser::PropertyTypeContext::NUMBER(size_t i) {
  return getToken(dzshader_parser::NUMBER, i);
}

tree::TerminalNode* dzshader_parser::PropertyTypeContext::COMMA() {
  return getToken(dzshader_parser::COMMA, 0);
}

tree::TerminalNode* dzshader_parser::PropertyTypeContext::RPAREN() {
  return getToken(dzshader_parser::RPAREN, 0);
}


size_t dzshader_parser::PropertyTypeContext::getRuleIndex() const {
  return dzshader_parser::RulePropertyType;
}


dzshader_parser::PropertyTypeContext* dzshader_parser::propertyType() {
  PropertyTypeContext *_localctx = _tracker.createInstance<PropertyTypeContext>(_ctx, getState());
  enterRule(_localctx, 10, dzshader_parser::RulePropertyType);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(100);
    _la = _input->LA(1);
    if (!(_la == dzshader_parser::TEXTURE_TYPE

    || _la == dzshader_parser::IDENTIFIER)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(106);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == dzshader_parser::LPAREN) {
      setState(101);
      match(dzshader_parser::LPAREN);
      setState(102);
      match(dzshader_parser::NUMBER);
      setState(103);
      match(dzshader_parser::COMMA);
      setState(104);
      match(dzshader_parser::NUMBER);
      setState(105);
      match(dzshader_parser::RPAREN);
    }

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ValueContext ------------------------------------------------------------------

dzshader_parser::ValueContext::ValueContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> dzshader_parser::ValueContext::NUMBER() {
  return getTokens(dzshader_parser::NUMBER);
}

tree::TerminalNode* dzshader_parser::ValueContext::NUMBER(size_t i) {
  return getToken(dzshader_parser::NUMBER, i);
}

tree::TerminalNode* dzshader_parser::ValueContext::IDENTIFIER() {
  return getToken(dzshader_parser::IDENTIFIER, 0);
}

tree::TerminalNode* dzshader_parser::ValueContext::STRING() {
  return getToken(dzshader_parser::STRING, 0);
}

tree::TerminalNode* dzshader_parser::ValueContext::LBRACE() {
  return getToken(dzshader_parser::LBRACE, 0);
}

tree::TerminalNode* dzshader_parser::ValueContext::RBRACE() {
  return getToken(dzshader_parser::RBRACE, 0);
}

tree::TerminalNode* dzshader_parser::ValueContext::LPAREN() {
  return getToken(dzshader_parser::LPAREN, 0);
}

tree::TerminalNode* dzshader_parser::ValueContext::RPAREN() {
  return getToken(dzshader_parser::RPAREN, 0);
}

std::vector<tree::TerminalNode *> dzshader_parser::ValueContext::COMMA() {
  return getTokens(dzshader_parser::COMMA);
}

tree::TerminalNode* dzshader_parser::ValueContext::COMMA(size_t i) {
  return getToken(dzshader_parser::COMMA, i);
}


size_t dzshader_parser::ValueContext::getRuleIndex() const {
  return dzshader_parser::RuleValue;
}


dzshader_parser::ValueContext* dzshader_parser::value() {
  ValueContext *_localctx = _tracker.createInstance<ValueContext>(_ctx, getState());
  enterRule(_localctx, 12, dzshader_parser::RuleValue);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(125);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case dzshader_parser::NUMBER: {
        enterOuterAlt(_localctx, 1);
        setState(108);
        match(dzshader_parser::NUMBER);
        break;
      }

      case dzshader_parser::IDENTIFIER: {
        enterOuterAlt(_localctx, 2);
        setState(109);
        match(dzshader_parser::IDENTIFIER);
        break;
      }

      case dzshader_parser::STRING: {
        enterOuterAlt(_localctx, 3);
        setState(110);
        match(dzshader_parser::STRING);
        setState(113);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == dzshader_parser::LBRACE) {
          setState(111);
          match(dzshader_parser::LBRACE);
          setState(112);
          match(dzshader_parser::RBRACE);
        }
        break;
      }

      case dzshader_parser::LPAREN: {
        enterOuterAlt(_localctx, 4);
        setState(115);
        match(dzshader_parser::LPAREN);
        setState(116);
        match(dzshader_parser::NUMBER);
        setState(121);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == dzshader_parser::COMMA) {
          setState(117);
          match(dzshader_parser::COMMA);
          setState(118);
          match(dzshader_parser::NUMBER);
          setState(123);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        setState(124);
        match(dzshader_parser::RPAREN);
        break;
      }

    default:
      throw NoViableAltException(this);
    }

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VariantContext ------------------------------------------------------------------

dzshader_parser::VariantContext::VariantContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::VariantContext::VARIANT() {
  return getToken(dzshader_parser::VARIANT, 0);
}

tree::TerminalNode* dzshader_parser::VariantContext::STRING() {
  return getToken(dzshader_parser::STRING, 0);
}

tree::TerminalNode* dzshader_parser::VariantContext::LBRACE() {
  return getToken(dzshader_parser::LBRACE, 0);
}

tree::TerminalNode* dzshader_parser::VariantContext::RBRACE() {
  return getToken(dzshader_parser::RBRACE, 0);
}

std::vector<dzshader_parser::ConstantsContext *> dzshader_parser::VariantContext::constants() {
  return getRuleContexts<dzshader_parser::ConstantsContext>();
}

dzshader_parser::ConstantsContext* dzshader_parser::VariantContext::constants(size_t i) {
  return getRuleContext<dzshader_parser::ConstantsContext>(i);
}

std::vector<dzshader_parser::ModulesContext *> dzshader_parser::VariantContext::modules() {
  return getRuleContexts<dzshader_parser::ModulesContext>();
}

dzshader_parser::ModulesContext* dzshader_parser::VariantContext::modules(size_t i) {
  return getRuleContext<dzshader_parser::ModulesContext>(i);
}

std::vector<dzshader_parser::DefinesContext *> dzshader_parser::VariantContext::defines() {
  return getRuleContexts<dzshader_parser::DefinesContext>();
}

dzshader_parser::DefinesContext* dzshader_parser::VariantContext::defines(size_t i) {
  return getRuleContext<dzshader_parser::DefinesContext>(i);
}


size_t dzshader_parser::VariantContext::getRuleIndex() const {
  return dzshader_parser::RuleVariant;
}


dzshader_parser::VariantContext* dzshader_parser::variant() {
  VariantContext *_localctx = _tracker.createInstance<VariantContext>(_ctx, getState());
  enterRule(_localctx, 14, dzshader_parser::RuleVariant);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(127);
    match(dzshader_parser::VARIANT);
    setState(128);
    match(dzshader_parser::STRING);
    setState(129);
    match(dzshader_parser::LBRACE);
    setState(135);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 14336) != 0)) {
      setState(133);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case dzshader_parser::CONSTANTS: {
          setState(130);
          constants();
          break;
        }

        case dzshader_parser::MODULES: {
          setState(131);
          modules();
          break;
        }

        case dzshader_parser::DEFINES: {
          setState(132);
          defines();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(137);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(138);
    match(dzshader_parser::RBRACE);

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstantsContext ------------------------------------------------------------------

dzshader_parser::ConstantsContext::ConstantsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::ConstantsContext::CONSTANTS() {
  return getToken(dzshader_parser::CONSTANTS, 0);
}

tree::TerminalNode* dzshader_parser::ConstantsContext::LBRACE() {
  return getToken(dzshader_parser::LBRACE, 0);
}

tree::TerminalNode* dzshader_parser::ConstantsContext::RBRACE() {
  return getToken(dzshader_parser::RBRACE, 0);
}

std::vector<tree::TerminalNode *> dzshader_parser::ConstantsContext::IDENTIFIER() {
  return getTokens(dzshader_parser::IDENTIFIER);
}

tree::TerminalNode* dzshader_parser::ConstantsContext::IDENTIFIER(size_t i) {
  return getToken(dzshader_parser::IDENTIFIER, i);
}

std::vector<tree::TerminalNode *> dzshader_parser::ConstantsContext::EQUAL() {
  return getTokens(dzshader_parser::EQUAL);
}

tree::TerminalNode* dzshader_parser::ConstantsContext::EQUAL(size_t i) {
  return getToken(dzshader_parser::EQUAL, i);
}

std::vector<tree::TerminalNode *> dzshader_parser::ConstantsContext::NUMBER() {
  return getTokens(dzshader_parser::NUMBER);
}

tree::TerminalNode* dzshader_parser::ConstantsContext::NUMBER(size_t i) {
  return getToken(dzshader_parser::NUMBER, i);
}


size_t dzshader_parser::ConstantsContext::getRuleIndex() const {
  return dzshader_parser::RuleConstants;
}


dzshader_parser::ConstantsContext* dzshader_parser::constants() {
  ConstantsContext *_localctx = _tracker.createInstance<ConstantsContext>(_ctx, getState());
  enterRule(_localctx, 16, dzshader_parser::RuleConstants);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(140);
    match(dzshader_parser::CONSTANTS);
    setState(141);
    match(dzshader_parser::LBRACE);
    setState(148);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == dzshader_parser::IDENTIFIER) {
      setState(142);
      match(dzshader_parser::IDENTIFIER);
      setState(143);
      match(dzshader_parser::IDENTIFIER);
      setState(144);
      match(dzshader_parser::EQUAL);
      setState(145);
      _la = _input->LA(1);
      if (!(_la == dzshader_parser::IDENTIFIER

      || _la == dzshader_parser::NUMBER)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(150);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(151);
    match(dzshader_parser::RBRACE);

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ModulesContext ------------------------------------------------------------------

dzshader_parser::ModulesContext::ModulesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::ModulesContext::MODULES() {
  return getToken(dzshader_parser::MODULES, 0);
}

tree::TerminalNode* dzshader_parser::ModulesContext::LBRACE() {
  return getToken(dzshader_parser::LBRACE, 0);
}

tree::TerminalNode* dzshader_parser::ModulesContext::RBRACE() {
  return getToken(dzshader_parser::RBRACE, 0);
}

std::vector<tree::TerminalNode *> dzshader_parser::ModulesContext::STRING() {
  return getTokens(dzshader_parser::STRING);
}

tree::TerminalNode* dzshader_parser::ModulesContext::STRING(size_t i) {
  return getToken(dzshader_parser::STRING, i);
}


size_t dzshader_parser::ModulesContext::getRuleIndex() const {
  return dzshader_parser::RuleModules;
}


dzshader_parser::ModulesContext* dzshader_parser::modules() {
  ModulesContext *_localctx = _tracker.createInstance<ModulesContext>(_ctx, getState());
  enterRule(_localctx, 18, dzshader_parser::RuleModules);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(153);
    match(dzshader_parser::MODULES);
    setState(154);
    match(dzshader_parser::LBRACE);
    setState(158);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == dzshader_parser::STRING) {
      setState(155);
      match(dzshader_parser::STRING);
      setState(160);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(161);
    match(dzshader_parser::RBRACE);

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DefinesContext ------------------------------------------------------------------

dzshader_parser::DefinesContext::DefinesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::DefinesContext::DEFINES() {
  return getToken(dzshader_parser::DEFINES, 0);
}

tree::TerminalNode* dzshader_parser::DefinesContext::LBRACE() {
  return getToken(dzshader_parser::LBRACE, 0);
}

tree::TerminalNode* dzshader_parser::DefinesContext::RBRACE() {
  return getToken(dzshader_parser::RBRACE, 0);
}

std::vector<tree::TerminalNode *> dzshader_parser::DefinesContext::IDENTIFIER() {
  return getTokens(dzshader_parser::IDENTIFIER);
}

tree::TerminalNode* dzshader_parser::DefinesContext::IDENTIFIER(size_t i) {
  return getToken(dzshader_parser::IDENTIFIER, i);
}

std::vector<tree::TerminalNode *> dzshader_parser::DefinesContext::EQUAL() {
  return getTokens(dzshader_parser::EQUAL);
}

tree::TerminalNode* dzshader_parser::DefinesContext::EQUAL(size_t i) {
  return getToken(dzshader_parser::EQUAL, i);
}

std::vector<tree::TerminalNode *> dzshader_parser::DefinesContext::NUMBER() {
  return getTokens(dzshader_parser::NUMBER);
}

tree::TerminalNode* dzshader_parser::DefinesContext::NUMBER(size_t i) {
  return getToken(dzshader_parser::NUMBER, i);
}

std::vector<tree::TerminalNode *> dzshader_parser::DefinesContext::STRING() {
  return getTokens(dzshader_parser::STRING);
}

tree::TerminalNode* dzshader_parser::DefinesContext::STRING(size_t i) {
  return getToken(dzshader_parser::STRING, i);
}


size_t dzshader_parser::DefinesContext::getRuleIndex() const {
  return dzshader_parser::RuleDefines;
}


dzshader_parser::DefinesContext* dzshader_parser::defines() {
  DefinesContext *_localctx = _tracker.createInstance<DefinesContext>(_ctx, getState());
  enterRule(_localctx, 20, dzshader_parser::RuleDefines);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(163);
    match(dzshader_parser::DEFINES);
    setState(164);
    match(dzshader_parser::LBRACE);
    setState(170);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == dzshader_parser::IDENTIFIER) {
      setState(165);
      match(dzshader_parser::IDENTIFIER);
      setState(166);
      match(dzshader_parser::EQUAL);
      setState(167);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 30064771072) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(172);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(173);
    match(dzshader_parser::RBRACE);

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SubshaderContext ------------------------------------------------------------------

dzshader_parser::SubshaderContext::SubshaderContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::SubshaderContext::SUBSHADER() {
  return getToken(dzshader_parser::SUBSHADER, 0);
}

tree::TerminalNode* dzshader_parser::SubshaderContext::LBRACE() {
  return getToken(dzshader_parser::LBRACE, 0);
}

tree::TerminalNode* dzshader_parser::SubshaderContext::RBRACE() {
  return getToken(dzshader_parser::RBRACE, 0);
}

std::vector<dzshader_parser::TagsContext *> dzshader_parser::SubshaderContext::tags() {
  return getRuleContexts<dzshader_parser::TagsContext>();
}

dzshader_parser::TagsContext* dzshader_parser::SubshaderContext::tags(size_t i) {
  return getRuleContext<dzshader_parser::TagsContext>(i);
}

std::vector<dzshader_parser::RequiresContext *> dzshader_parser::SubshaderContext::requires_() {
  return getRuleContexts<dzshader_parser::RequiresContext>();
}

dzshader_parser::RequiresContext* dzshader_parser::SubshaderContext::requires_(size_t i) {
  return getRuleContext<dzshader_parser::RequiresContext>(i);
}

std::vector<dzshader_parser::StateContext *> dzshader_parser::SubshaderContext::state() {
  return getRuleContexts<dzshader_parser::StateContext>();
}

dzshader_parser::StateContext* dzshader_parser::SubshaderContext::state(size_t i) {
  return getRuleContext<dzshader_parser::StateContext>(i);
}

std::vector<dzshader_parser::CommonContext *> dzshader_parser::SubshaderContext::common() {
  return getRuleContexts<dzshader_parser::CommonContext>();
}

dzshader_parser::CommonContext* dzshader_parser::SubshaderContext::common(size_t i) {
  return getRuleContext<dzshader_parser::CommonContext>(i);
}

std::vector<dzshader_parser::SurfaceContext *> dzshader_parser::SubshaderContext::surface() {
  return getRuleContexts<dzshader_parser::SurfaceContext>();
}

dzshader_parser::SurfaceContext* dzshader_parser::SubshaderContext::surface(size_t i) {
  return getRuleContext<dzshader_parser::SurfaceContext>(i);
}

std::vector<dzshader_parser::PassContext *> dzshader_parser::SubshaderContext::pass() {
  return getRuleContexts<dzshader_parser::PassContext>();
}

dzshader_parser::PassContext* dzshader_parser::SubshaderContext::pass(size_t i) {
  return getRuleContext<dzshader_parser::PassContext>(i);
}


size_t dzshader_parser::SubshaderContext::getRuleIndex() const {
  return dzshader_parser::RuleSubshader;
}


dzshader_parser::SubshaderContext* dzshader_parser::subshader() {
  SubshaderContext *_localctx = _tracker.createInstance<SubshaderContext>(_ctx, getState());
  enterRule(_localctx, 22, dzshader_parser::RuleSubshader);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(175);
    match(dzshader_parser::SUBSHADER);
    setState(176);
    match(dzshader_parser::LBRACE);
    setState(185);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1609580656) != 0)) {
      setState(183);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case dzshader_parser::TAGS: {
          setState(177);
          tags();
          break;
        }

        case dzshader_parser::REQUIRES: {
          setState(178);
          requires_();
          break;
        }

        case dzshader_parser::CULL:
        case dzshader_parser::FRONT_FACE:
        case dzshader_parser::ZWRITE:
        case dzshader_parser::ZTEST:
        case dzshader_parser::DEPTH_BIAS:
        case dzshader_parser::BLEND:
        case dzshader_parser::BLEND_OP:
        case dzshader_parser::COLOR_MASK:
        case dzshader_parser::STENCIL: {
          setState(179);
          state();
          break;
        }

        case dzshader_parser::SLANG_INCLUDE: {
          setState(180);
          common();
          break;
        }

        case dzshader_parser::SURFACE: {
          setState(181);
          surface();
          break;
        }

        case dzshader_parser::PASS: {
          setState(182);
          pass();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(187);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(188);
    match(dzshader_parser::RBRACE);

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TagsContext ------------------------------------------------------------------

dzshader_parser::TagsContext::TagsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::TagsContext::TAGS() {
  return getToken(dzshader_parser::TAGS, 0);
}

tree::TerminalNode* dzshader_parser::TagsContext::LBRACE() {
  return getToken(dzshader_parser::LBRACE, 0);
}

tree::TerminalNode* dzshader_parser::TagsContext::RBRACE() {
  return getToken(dzshader_parser::RBRACE, 0);
}

std::vector<tree::TerminalNode *> dzshader_parser::TagsContext::STRING() {
  return getTokens(dzshader_parser::STRING);
}

tree::TerminalNode* dzshader_parser::TagsContext::STRING(size_t i) {
  return getToken(dzshader_parser::STRING, i);
}

std::vector<tree::TerminalNode *> dzshader_parser::TagsContext::EQUAL() {
  return getTokens(dzshader_parser::EQUAL);
}

tree::TerminalNode* dzshader_parser::TagsContext::EQUAL(size_t i) {
  return getToken(dzshader_parser::EQUAL, i);
}


size_t dzshader_parser::TagsContext::getRuleIndex() const {
  return dzshader_parser::RuleTags;
}


dzshader_parser::TagsContext* dzshader_parser::tags() {
  TagsContext *_localctx = _tracker.createInstance<TagsContext>(_ctx, getState());
  enterRule(_localctx, 24, dzshader_parser::RuleTags);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(190);
    match(dzshader_parser::TAGS);
    setState(191);
    match(dzshader_parser::LBRACE);
    setState(197);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == dzshader_parser::STRING) {
      setState(192);
      match(dzshader_parser::STRING);
      setState(193);
      match(dzshader_parser::EQUAL);
      setState(194);
      match(dzshader_parser::STRING);
      setState(199);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(200);
    match(dzshader_parser::RBRACE);

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RequiresContext ------------------------------------------------------------------

dzshader_parser::RequiresContext::RequiresContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::RequiresContext::REQUIRES() {
  return getToken(dzshader_parser::REQUIRES, 0);
}

std::vector<tree::TerminalNode *> dzshader_parser::RequiresContext::IDENTIFIER() {
  return getTokens(dzshader_parser::IDENTIFIER);
}

tree::TerminalNode* dzshader_parser::RequiresContext::IDENTIFIER(size_t i) {
  return getToken(dzshader_parser::IDENTIFIER, i);
}

std::vector<tree::TerminalNode *> dzshader_parser::RequiresContext::COMMA() {
  return getTokens(dzshader_parser::COMMA);
}

tree::TerminalNode* dzshader_parser::RequiresContext::COMMA(size_t i) {
  return getToken(dzshader_parser::COMMA, i);
}


size_t dzshader_parser::RequiresContext::getRuleIndex() const {
  return dzshader_parser::RuleRequires;
}


dzshader_parser::RequiresContext* dzshader_parser::requires_() {
  RequiresContext *_localctx = _tracker.createInstance<RequiresContext>(_ctx, getState());
  enterRule(_localctx, 26, dzshader_parser::RuleRequires);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(202);
    match(dzshader_parser::REQUIRES);
    setState(203);
    match(dzshader_parser::IDENTIFIER);
    setState(208);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == dzshader_parser::COMMA) {
      setState(204);
      match(dzshader_parser::COMMA);
      setState(205);
      match(dzshader_parser::IDENTIFIER);
      setState(210);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CommonContext ------------------------------------------------------------------

dzshader_parser::CommonContext::CommonContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::CommonContext::SLANG_INCLUDE() {
  return getToken(dzshader_parser::SLANG_INCLUDE, 0);
}

tree::TerminalNode* dzshader_parser::CommonContext::PROGRAM_END() {
  return getToken(dzshader_parser::PROGRAM_END, 0);
}

std::vector<tree::TerminalNode *> dzshader_parser::CommonContext::PROGRAM_TEXT() {
  return getTokens(dzshader_parser::PROGRAM_TEXT);
}

tree::TerminalNode* dzshader_parser::CommonContext::PROGRAM_TEXT(size_t i) {
  return getToken(dzshader_parser::PROGRAM_TEXT, i);
}


size_t dzshader_parser::CommonContext::getRuleIndex() const {
  return dzshader_parser::RuleCommon;
}


dzshader_parser::CommonContext* dzshader_parser::common() {
  CommonContext *_localctx = _tracker.createInstance<CommonContext>(_ctx, getState());
  enterRule(_localctx, 28, dzshader_parser::RuleCommon);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(211);
    match(dzshader_parser::SLANG_INCLUDE);
    setState(215);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == dzshader_parser::PROGRAM_TEXT) {
      setState(212);
      match(dzshader_parser::PROGRAM_TEXT);
      setState(217);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(218);
    match(dzshader_parser::PROGRAM_END);

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ProgramContext ------------------------------------------------------------------

dzshader_parser::ProgramContext::ProgramContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::ProgramContext::SLANG_PROGRAM() {
  return getToken(dzshader_parser::SLANG_PROGRAM, 0);
}

tree::TerminalNode* dzshader_parser::ProgramContext::PROGRAM_END() {
  return getToken(dzshader_parser::PROGRAM_END, 0);
}

std::vector<tree::TerminalNode *> dzshader_parser::ProgramContext::PROGRAM_TEXT() {
  return getTokens(dzshader_parser::PROGRAM_TEXT);
}

tree::TerminalNode* dzshader_parser::ProgramContext::PROGRAM_TEXT(size_t i) {
  return getToken(dzshader_parser::PROGRAM_TEXT, i);
}


size_t dzshader_parser::ProgramContext::getRuleIndex() const {
  return dzshader_parser::RuleProgram;
}


dzshader_parser::ProgramContext* dzshader_parser::program() {
  ProgramContext *_localctx = _tracker.createInstance<ProgramContext>(_ctx, getState());
  enterRule(_localctx, 30, dzshader_parser::RuleProgram);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(220);
    match(dzshader_parser::SLANG_PROGRAM);
    setState(224);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == dzshader_parser::PROGRAM_TEXT) {
      setState(221);
      match(dzshader_parser::PROGRAM_TEXT);
      setState(226);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(227);
    match(dzshader_parser::PROGRAM_END);

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SurfaceContext ------------------------------------------------------------------

dzshader_parser::SurfaceContext::SurfaceContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::SurfaceContext::SURFACE() {
  return getToken(dzshader_parser::SURFACE, 0);
}

tree::TerminalNode* dzshader_parser::SurfaceContext::LBRACE() {
  return getToken(dzshader_parser::LBRACE, 0);
}

tree::TerminalNode* dzshader_parser::SurfaceContext::RBRACE() {
  return getToken(dzshader_parser::RBRACE, 0);
}

std::vector<tree::TerminalNode *> dzshader_parser::SurfaceContext::MODEL() {
  return getTokens(dzshader_parser::MODEL);
}

tree::TerminalNode* dzshader_parser::SurfaceContext::MODEL(size_t i) {
  return getToken(dzshader_parser::MODEL, i);
}

std::vector<tree::TerminalNode *> dzshader_parser::SurfaceContext::IDENTIFIER() {
  return getTokens(dzshader_parser::IDENTIFIER);
}

tree::TerminalNode* dzshader_parser::SurfaceContext::IDENTIFIER(size_t i) {
  return getToken(dzshader_parser::IDENTIFIER, i);
}

std::vector<tree::TerminalNode *> dzshader_parser::SurfaceContext::ENTRY() {
  return getTokens(dzshader_parser::ENTRY);
}

tree::TerminalNode* dzshader_parser::SurfaceContext::ENTRY(size_t i) {
  return getToken(dzshader_parser::ENTRY, i);
}

std::vector<dzshader_parser::StateContext *> dzshader_parser::SurfaceContext::state() {
  return getRuleContexts<dzshader_parser::StateContext>();
}

dzshader_parser::StateContext* dzshader_parser::SurfaceContext::state(size_t i) {
  return getRuleContext<dzshader_parser::StateContext>(i);
}

std::vector<dzshader_parser::ProgramContext *> dzshader_parser::SurfaceContext::program() {
  return getRuleContexts<dzshader_parser::ProgramContext>();
}

dzshader_parser::ProgramContext* dzshader_parser::SurfaceContext::program(size_t i) {
  return getRuleContext<dzshader_parser::ProgramContext>(i);
}


size_t dzshader_parser::SurfaceContext::getRuleIndex() const {
  return dzshader_parser::RuleSurface;
}


dzshader_parser::SurfaceContext* dzshader_parser::surface() {
  SurfaceContext *_localctx = _tracker.createInstance<SurfaceContext>(_ctx, getState());
  enterRule(_localctx, 32, dzshader_parser::RuleSurface);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(229);
    match(dzshader_parser::SURFACE);
    setState(230);
    match(dzshader_parser::LBRACE);
    setState(239);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1072694016) != 0)) {
      setState(237);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case dzshader_parser::MODEL: {
          setState(231);
          match(dzshader_parser::MODEL);
          setState(232);
          match(dzshader_parser::IDENTIFIER);
          break;
        }

        case dzshader_parser::ENTRY: {
          setState(233);
          match(dzshader_parser::ENTRY);
          setState(234);
          match(dzshader_parser::IDENTIFIER);
          break;
        }

        case dzshader_parser::CULL:
        case dzshader_parser::FRONT_FACE:
        case dzshader_parser::ZWRITE:
        case dzshader_parser::ZTEST:
        case dzshader_parser::DEPTH_BIAS:
        case dzshader_parser::BLEND:
        case dzshader_parser::BLEND_OP:
        case dzshader_parser::COLOR_MASK:
        case dzshader_parser::STENCIL: {
          setState(235);
          state();
          break;
        }

        case dzshader_parser::SLANG_PROGRAM: {
          setState(236);
          program();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(241);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(242);
    match(dzshader_parser::RBRACE);

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PassContext ------------------------------------------------------------------

dzshader_parser::PassContext::PassContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::PassContext::PASS() {
  return getToken(dzshader_parser::PASS, 0);
}

tree::TerminalNode* dzshader_parser::PassContext::LBRACE() {
  return getToken(dzshader_parser::LBRACE, 0);
}

tree::TerminalNode* dzshader_parser::PassContext::RBRACE() {
  return getToken(dzshader_parser::RBRACE, 0);
}

std::vector<tree::TerminalNode *> dzshader_parser::PassContext::NAME() {
  return getTokens(dzshader_parser::NAME);
}

tree::TerminalNode* dzshader_parser::PassContext::NAME(size_t i) {
  return getToken(dzshader_parser::NAME, i);
}

std::vector<tree::TerminalNode *> dzshader_parser::PassContext::STRING() {
  return getTokens(dzshader_parser::STRING);
}

tree::TerminalNode* dzshader_parser::PassContext::STRING(size_t i) {
  return getToken(dzshader_parser::STRING, i);
}

std::vector<dzshader_parser::TagsContext *> dzshader_parser::PassContext::tags() {
  return getRuleContexts<dzshader_parser::TagsContext>();
}

dzshader_parser::TagsContext* dzshader_parser::PassContext::tags(size_t i) {
  return getRuleContext<dzshader_parser::TagsContext>(i);
}

std::vector<dzshader_parser::StageContext *> dzshader_parser::PassContext::stage() {
  return getRuleContexts<dzshader_parser::StageContext>();
}

dzshader_parser::StageContext* dzshader_parser::PassContext::stage(size_t i) {
  return getRuleContext<dzshader_parser::StageContext>(i);
}

std::vector<dzshader_parser::RequiresContext *> dzshader_parser::PassContext::requires_() {
  return getRuleContexts<dzshader_parser::RequiresContext>();
}

dzshader_parser::RequiresContext* dzshader_parser::PassContext::requires_(size_t i) {
  return getRuleContext<dzshader_parser::RequiresContext>(i);
}

std::vector<dzshader_parser::StateContext *> dzshader_parser::PassContext::state() {
  return getRuleContexts<dzshader_parser::StateContext>();
}

dzshader_parser::StateContext* dzshader_parser::PassContext::state(size_t i) {
  return getRuleContext<dzshader_parser::StateContext>(i);
}

std::vector<dzshader_parser::ProgramContext *> dzshader_parser::PassContext::program() {
  return getRuleContexts<dzshader_parser::ProgramContext>();
}

dzshader_parser::ProgramContext* dzshader_parser::PassContext::program(size_t i) {
  return getRuleContext<dzshader_parser::ProgramContext>(i);
}


size_t dzshader_parser::PassContext::getRuleIndex() const {
  return dzshader_parser::RulePass;
}


dzshader_parser::PassContext* dzshader_parser::pass() {
  PassContext *_localctx = _tracker.createInstance<PassContext>(_ctx, getState());
  enterRule(_localctx, 34, dzshader_parser::RulePass);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(244);
    match(dzshader_parser::PASS);
    setState(245);
    match(dzshader_parser::LBRACE);
    setState(255);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1073725632) != 0)) {
      setState(253);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case dzshader_parser::NAME: {
          setState(246);
          match(dzshader_parser::NAME);
          setState(247);
          match(dzshader_parser::STRING);
          break;
        }

        case dzshader_parser::TAGS: {
          setState(248);
          tags();
          break;
        }

        case dzshader_parser::VERTEX:
        case dzshader_parser::FRAGMENT:
        case dzshader_parser::TASK:
        case dzshader_parser::MESH:
        case dzshader_parser::COMPUTE: {
          setState(249);
          stage();
          break;
        }

        case dzshader_parser::REQUIRES: {
          setState(250);
          requires_();
          break;
        }

        case dzshader_parser::CULL:
        case dzshader_parser::FRONT_FACE:
        case dzshader_parser::ZWRITE:
        case dzshader_parser::ZTEST:
        case dzshader_parser::DEPTH_BIAS:
        case dzshader_parser::BLEND:
        case dzshader_parser::BLEND_OP:
        case dzshader_parser::COLOR_MASK:
        case dzshader_parser::STENCIL: {
          setState(251);
          state();
          break;
        }

        case dzshader_parser::SLANG_PROGRAM: {
          setState(252);
          program();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(257);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(258);
    match(dzshader_parser::RBRACE);

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StageContext ------------------------------------------------------------------

dzshader_parser::StageContext::StageContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::StageContext::IDENTIFIER() {
  return getToken(dzshader_parser::IDENTIFIER, 0);
}

tree::TerminalNode* dzshader_parser::StageContext::VERTEX() {
  return getToken(dzshader_parser::VERTEX, 0);
}

tree::TerminalNode* dzshader_parser::StageContext::FRAGMENT() {
  return getToken(dzshader_parser::FRAGMENT, 0);
}

tree::TerminalNode* dzshader_parser::StageContext::TASK() {
  return getToken(dzshader_parser::TASK, 0);
}

tree::TerminalNode* dzshader_parser::StageContext::MESH() {
  return getToken(dzshader_parser::MESH, 0);
}

tree::TerminalNode* dzshader_parser::StageContext::COMPUTE() {
  return getToken(dzshader_parser::COMPUTE, 0);
}


size_t dzshader_parser::StageContext::getRuleIndex() const {
  return dzshader_parser::RuleStage;
}


dzshader_parser::StageContext* dzshader_parser::stage() {
  StageContext *_localctx = _tracker.createInstance<StageContext>(_ctx, getState());
  enterRule(_localctx, 36, dzshader_parser::RuleStage);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(260);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1015808) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(261);
    match(dzshader_parser::IDENTIFIER);

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StateContext ------------------------------------------------------------------

dzshader_parser::StateContext::StateContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::StateContext::CULL() {
  return getToken(dzshader_parser::CULL, 0);
}

std::vector<dzshader_parser::StateValueContext *> dzshader_parser::StateContext::stateValue() {
  return getRuleContexts<dzshader_parser::StateValueContext>();
}

dzshader_parser::StateValueContext* dzshader_parser::StateContext::stateValue(size_t i) {
  return getRuleContext<dzshader_parser::StateValueContext>(i);
}

tree::TerminalNode* dzshader_parser::StateContext::FRONT_FACE() {
  return getToken(dzshader_parser::FRONT_FACE, 0);
}

tree::TerminalNode* dzshader_parser::StateContext::ZWRITE() {
  return getToken(dzshader_parser::ZWRITE, 0);
}

tree::TerminalNode* dzshader_parser::StateContext::ZTEST() {
  return getToken(dzshader_parser::ZTEST, 0);
}

tree::TerminalNode* dzshader_parser::StateContext::DEPTH_BIAS() {
  return getToken(dzshader_parser::DEPTH_BIAS, 0);
}

tree::TerminalNode* dzshader_parser::StateContext::BLEND() {
  return getToken(dzshader_parser::BLEND, 0);
}

tree::TerminalNode* dzshader_parser::StateContext::COMMA() {
  return getToken(dzshader_parser::COMMA, 0);
}

tree::TerminalNode* dzshader_parser::StateContext::BLEND_OP() {
  return getToken(dzshader_parser::BLEND_OP, 0);
}

tree::TerminalNode* dzshader_parser::StateContext::COLOR_MASK() {
  return getToken(dzshader_parser::COLOR_MASK, 0);
}

tree::TerminalNode* dzshader_parser::StateContext::NUMBER() {
  return getToken(dzshader_parser::NUMBER, 0);
}

tree::TerminalNode* dzshader_parser::StateContext::STENCIL() {
  return getToken(dzshader_parser::STENCIL, 0);
}

tree::TerminalNode* dzshader_parser::StateContext::LBRACE() {
  return getToken(dzshader_parser::LBRACE, 0);
}

tree::TerminalNode* dzshader_parser::StateContext::RBRACE() {
  return getToken(dzshader_parser::RBRACE, 0);
}

std::vector<tree::TerminalNode *> dzshader_parser::StateContext::IDENTIFIER() {
  return getTokens(dzshader_parser::IDENTIFIER);
}

tree::TerminalNode* dzshader_parser::StateContext::IDENTIFIER(size_t i) {
  return getToken(dzshader_parser::IDENTIFIER, i);
}


size_t dzshader_parser::StateContext::getRuleIndex() const {
  return dzshader_parser::RuleState;
}


dzshader_parser::StateContext* dzshader_parser::state() {
  StateContext *_localctx = _tracker.createInstance<StateContext>(_ctx, getState());
  enterRule(_localctx, 38, dzshader_parser::RuleState);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(309);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case dzshader_parser::CULL: {
        enterOuterAlt(_localctx, 1);
        setState(263);
        match(dzshader_parser::CULL);
        setState(264);
        stateValue();
        break;
      }

      case dzshader_parser::FRONT_FACE: {
        enterOuterAlt(_localctx, 2);
        setState(265);
        match(dzshader_parser::FRONT_FACE);
        setState(266);
        stateValue();
        break;
      }

      case dzshader_parser::ZWRITE: {
        enterOuterAlt(_localctx, 3);
        setState(267);
        match(dzshader_parser::ZWRITE);
        setState(268);
        stateValue();
        break;
      }

      case dzshader_parser::ZTEST: {
        enterOuterAlt(_localctx, 4);
        setState(269);
        match(dzshader_parser::ZTEST);
        setState(270);
        stateValue();
        break;
      }

      case dzshader_parser::DEPTH_BIAS: {
        enterOuterAlt(_localctx, 5);
        setState(271);
        match(dzshader_parser::DEPTH_BIAS);
        setState(272);
        stateValue();
        setState(273);
        stateValue();
        setState(275);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 562640715776) != 0)) {
          setState(274);
          stateValue();
        }
        break;
      }

      case dzshader_parser::BLEND: {
        enterOuterAlt(_localctx, 6);
        setState(277);
        match(dzshader_parser::BLEND);
        setState(278);
        stateValue();
        setState(286);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 562640715776) != 0)) {
          setState(279);
          stateValue();
          setState(284);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == dzshader_parser::COMMA) {
            setState(280);
            match(dzshader_parser::COMMA);
            setState(281);
            stateValue();
            setState(282);
            stateValue();
          }
        }
        break;
      }

      case dzshader_parser::BLEND_OP: {
        enterOuterAlt(_localctx, 7);
        setState(288);
        match(dzshader_parser::BLEND_OP);
        setState(289);
        stateValue();
        setState(292);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == dzshader_parser::COMMA) {
          setState(290);
          match(dzshader_parser::COMMA);
          setState(291);
          stateValue();
        }
        break;
      }

      case dzshader_parser::COLOR_MASK: {
        enterOuterAlt(_localctx, 8);
        setState(294);
        match(dzshader_parser::COLOR_MASK);
        setState(295);
        stateValue();
        setState(297);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == dzshader_parser::NUMBER) {
          setState(296);
          match(dzshader_parser::NUMBER);
        }
        break;
      }

      case dzshader_parser::STENCIL: {
        enterOuterAlt(_localctx, 9);
        setState(299);
        match(dzshader_parser::STENCIL);
        setState(300);
        match(dzshader_parser::LBRACE);
        setState(305);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == dzshader_parser::IDENTIFIER) {
          setState(301);
          match(dzshader_parser::IDENTIFIER);
          setState(302);
          stateValue();
          setState(307);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        setState(308);
        match(dzshader_parser::RBRACE);
        break;
      }

    default:
      throw NoViableAltException(this);
    }

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StateValueContext ------------------------------------------------------------------

dzshader_parser::StateValueContext::StateValueContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* dzshader_parser::StateValueContext::IDENTIFIER() {
  return getToken(dzshader_parser::IDENTIFIER, 0);
}

tree::TerminalNode* dzshader_parser::StateValueContext::NUMBER() {
  return getToken(dzshader_parser::NUMBER, 0);
}

tree::TerminalNode* dzshader_parser::StateValueContext::LBRACKET() {
  return getToken(dzshader_parser::LBRACKET, 0);
}

tree::TerminalNode* dzshader_parser::StateValueContext::RBRACKET() {
  return getToken(dzshader_parser::RBRACKET, 0);
}


size_t dzshader_parser::StateValueContext::getRuleIndex() const {
  return dzshader_parser::RuleStateValue;
}


dzshader_parser::StateValueContext* dzshader_parser::stateValue() {
  StateValueContext *_localctx = _tracker.createInstance<StateValueContext>(_ctx, getState());
  enterRule(_localctx, 40, dzshader_parser::RuleStateValue);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(316);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case dzshader_parser::IDENTIFIER: {
        enterOuterAlt(_localctx, 1);
        setState(311);
        match(dzshader_parser::IDENTIFIER);
        break;
      }

      case dzshader_parser::NUMBER: {
        enterOuterAlt(_localctx, 2);
        setState(312);
        match(dzshader_parser::NUMBER);
        break;
      }

      case dzshader_parser::LBRACKET: {
        enterOuterAlt(_localctx, 3);
        setState(313);
        match(dzshader_parser::LBRACKET);
        setState(314);
        match(dzshader_parser::IDENTIFIER);
        setState(315);
        match(dzshader_parser::RBRACKET);
        break;
      }

    default:
      throw NoViableAltException(this);
    }

  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

void dzshader_parser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  dzshader_parserParserInitialize();
#else
  ::antlr4::internal::call_once(dzshader_parserParserOnceFlag, dzshader_parserParserInitialize);
#endif
}
