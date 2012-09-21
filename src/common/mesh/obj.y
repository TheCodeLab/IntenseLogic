%token SLASH
%token NEWLINE
%token INTEGER
%token REAL

%token FACE
%token VERTEX
%token TEXCOORD
%token NORMAL
%token FREEFORM

%%

facepoint     : INTEGER SLASH INTEGER SLASH INTEGER
              | INTEGER SLASH SLASH INTEGER
              | INTEGER SLASH INTEGER
              | INTEGER
              ;

facedesc      : FACE facepoint facepoint facepoint
              | facedesc facepoint
              ;

vertexdesc    : VERTEX REAL REAL REAL
              | vertexdesc REAL
              ;

texcoorddesc  : TEXCOORD REAL
              | texcoorddesc REAL
              | texcoorddesc REAL
              ;

normaldesc    : NORMAL REAL REAL REAL 
              ;

freeformdesc  : FREEFORM REAL
              | freeformdesc REAL
              | freeformdesc REAL
              ;
