lexer grammar dzshader_lexer;

@members {
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
}

SHADER: 'Shader';
PROPERTIES: 'Properties';
SUBSHADER: 'SubShader';
PASS: 'Pass';
SURFACE: 'Surface';
TAGS: 'Tags';
NAME: 'Name';
MODEL: 'Model';
ENTRY: 'Entry';
VARIANT: 'Variant';
CONSTANTS: 'Constants';
MODULES: 'Modules';
DEFINES: 'Defines';
REQUIRES: 'Requires';
VERTEX: 'Vertex';
FRAGMENT: 'Fragment';
TASK: 'Task';
MESH: 'Mesh';
COMPUTE: 'Compute';
CULL: 'Cull';
FRONT_FACE: 'FrontFace';
ZWRITE: 'ZWrite';
ZTEST: 'ZTest';
DEPTH_BIAS: 'DepthBias';
BLEND: 'Blend';
BLEND_OP: 'BlendOp';
COLOR_MASK: 'ColorMask';
STENCIL: 'Stencil';
SLANG_PROGRAM: 'SLANGPROGRAM' [ \t]* '\r'? '\n' -> pushMode(PROGRAM);
SLANG_INCLUDE: 'SLANGINCLUDE' [ \t]* '\r'? '\n' -> pushMode(PROGRAM);
TEXTURE_TYPE: '2DArray' | '2D' | '3D';
IDENTIFIER: [a-zA-Z_] [a-zA-Z_0-9]*;
NUMBER: [+-]? ([0-9]+ ('.' [0-9]*)? | '.' [0-9]+) ([eE] [+-]? [0-9]+)?;
STRING: '"' ('\\' . | ~["\\\r\n])* '"';
LBRACE: '{';
RBRACE: '}';
LPAREN: '(';
RPAREN: ')';
LBRACKET: '[';
RBRACKET: ']';
COMMA: ',';
EQUAL: '=';
SEMICOLON: ';' -> skip;
LINE_COMMENT: '//' ~[\r\n]* -> skip;
BLOCK_COMMENT: '/*' .*? '*/' -> skip;
WHITESPACE: [ \t\r\n]+ -> skip;

mode PROGRAM;
PROGRAM_END: {isProgramEnd()}? [ \t]* 'ENDSLANG' [ \t]* ('\r'? '\n' | EOF) -> popMode;
PROGRAM_LINE_COMMENT: '//' ('\\' '\r'? '\n' | ~[\r\n])* -> type(PROGRAM_TEXT);
PROGRAM_BLOCK_COMMENT: '/*' .*? '*/' -> type(PROGRAM_TEXT);
PROGRAM_STRING: '"' ('\\' . | ~["\\])* '"' -> type(PROGRAM_TEXT);
PROGRAM_CHARACTER: '\'' ('\\' . | ~['\\])* '\'' -> type(PROGRAM_TEXT);
PROGRAM_RAW_START: 'R"' ~[("\r\n]* '('
    {rawDelimiter = getText().substr(2, getText().size() - 3);} -> more, pushMode(RAW_STRING);
PROGRAM_TEXT: ({!isProgramEnd() && !isProgramSpecial()}? .)+;

mode RAW_STRING;
RAW_STRING_END: {isRawStringEnd()}? ')' ~["]* '"' -> type(PROGRAM_TEXT), popMode;
RAW_STRING_TEXT: ({!isRawStringEnd()}? .)+ -> more;
