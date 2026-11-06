parser grammar dzshader_parser;
options { tokenVocab = dzshader_lexer; }

file: SHADER STRING LBRACE (properties | variant | subshader)* RBRACE EOF;
properties: PROPERTIES LBRACE property* RBRACE;
property: attribute* IDENTIFIER LPAREN STRING COMMA propertyType RPAREN EQUAL value;
attribute: LBRACKET IDENTIFIER (LPAREN attributeArgument (COMMA attributeArgument)* RPAREN)? RBRACKET;
attributeArgument: IDENTIFIER | NUMBER | STRING;
propertyType: (IDENTIFIER | TEXTURE_TYPE) (LPAREN NUMBER COMMA NUMBER RPAREN)?;
value: NUMBER | IDENTIFIER | STRING (LBRACE RBRACE)? | LPAREN NUMBER (COMMA NUMBER)* RPAREN;
variant: VARIANT STRING LBRACE (constants | modules | defines)* RBRACE;
constants: CONSTANTS LBRACE (IDENTIFIER IDENTIFIER EQUAL (NUMBER | IDENTIFIER))* RBRACE;
modules: MODULES LBRACE STRING* RBRACE;
defines: DEFINES LBRACE (IDENTIFIER EQUAL (NUMBER | IDENTIFIER | STRING))* RBRACE;
subshader: SUBSHADER LBRACE (tags | requires | state | common | surface | pass)* RBRACE;
tags: TAGS LBRACE (STRING EQUAL STRING)* RBRACE;
requires: REQUIRES IDENTIFIER (COMMA IDENTIFIER)*;
common: SLANG_INCLUDE PROGRAM_TEXT* PROGRAM_END;
program: SLANG_PROGRAM PROGRAM_TEXT* PROGRAM_END;
surface: SURFACE LBRACE (MODEL IDENTIFIER | ENTRY IDENTIFIER | state | program)* RBRACE;
pass: PASS LBRACE (NAME STRING | tags | stage | requires | state | program)* RBRACE;
stage: (VERTEX | FRAGMENT | TASK | MESH | COMPUTE) IDENTIFIER;
state:
      CULL stateValue
    | FRONT_FACE stateValue
    | ZWRITE stateValue
    | ZTEST stateValue
    | DEPTH_BIAS stateValue stateValue stateValue?
    | BLEND stateValue (stateValue (COMMA stateValue stateValue)?)?
    | BLEND_OP stateValue (COMMA stateValue)?
    | COLOR_MASK stateValue NUMBER?
    | STENCIL LBRACE (IDENTIFIER stateValue)* RBRACE;
stateValue: IDENTIFIER | NUMBER | LBRACKET IDENTIFIER RBRACKET;
