1 'verbose !
/* initial network callbacks - just record caller */
cin !
cout !
{ drop } 'disconnected !
{ drop } 'connected !
{ drop } 'incoming !
{ drop Null } 'outgoing !

{names swap search} 'defined !
{[ ] 'Namespace new} 'local !
{negative this swap element} 'nover !
{2 nover} 'over !
{3 nover} '2over !
{over over} '2dup !
{{0}{1} if} 'not !

/* nrot: < xn xn-1 .. x1 n -- xn-1 .. x1 xn > */
{1 + negative this this -2 element element swap this swap delete drop } 'nrot !

/* rot: < a b c -- b c a > */
{3 nrot} 'rot !

10 chr 'lf !
/* [ ] 'connections ! */

{ $cout swap sreplace drop } 'emit !
{ $lf + emit } 'emitln !
{ $verbose 1 > {emitln} {drop} if } 'vemitln !
{ $verbose {emit} {drop} if } 'vemit !

/* addfn < name code -- > */
{ swap dup dup scopes swap search Null == {"Adding "} {"Redefining "} if swap + vemitln !  } 'addfn !

{ 'Regexp new } 'regexp !
{ 'Dict   new } 'dict   !
/* { 'Set    new } 'nset   ! */
{ 'Hash   new } 'hash   !
{ 'File   new } 'file   !

"Running boot.Ch..." vemitln

'emptycase { drop drop "" } addfn
'singlecase { drop 0 element } addfn

/* < list sep -- string > */
'multicase { local ( 'sep ! removefirst swap iterator 'it ! {$sep + $it next +} {$it more} while ) } addfn
'multicase { swap removefirst { 2over + swap removefirst rot swap + } {over} while swap drop swap drop } addfn

/* < list -- list[1..$] list[0] > */
'removefirst { dup 0 element swap 0 delete swap } addfn

/* Concat a list of strings < list separator -- string > */
'join { over length dup 0 == {drop emptycase} {1 == {singlecase} {multicase} if } if } addfn

'filetolist { [ ] swap iterator 'iter !  { $iter next append } { $iter more } while } addfn

/* < File sep -- string > */
'joinf { swap filetolist swap join } addfn

'parsefile
{ [ ] swap append 1 append file 10 chr joinf "{" swap + "}" + parse }
addfn

'include
{ "Processing " over + vemitln parsefile dup {"WARNING - parser leftovers: " emit emitln} {drop} if exec "done" vemitln } 
addfn

"lib.Ch" include

"boot.Ch done" emitln

/* real network primitives */
"" 'inbuffer !
'incoming {  -1 element " " join $inbuffer swap add rpush [ rpop parse 'inbuffer ! ] exec } addfn

