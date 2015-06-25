/* inttostr  < int -- str > */

'inttostr { 
    "" swap
    { dup 10 %
      "0123456789" swap element
      rot + swap
      10 /
    } { dup }
  while
  drop
} addfn

'isInteger { type 0   type == } addfn
'isString  { type ""  type == } addfn
'isList    { type []  type == } addfn
'isSymbol  { type names 'isSymbol element type == } addfn

'rdup { that -1 element } addfn
'rdrop { rpop drop } addfn

/* foreach < list code -- ... > */
'foreach {
    local dup rpush (
        'code !
        iterator 'it !
    )
        { rdup ( $it next $code ) exec }
        { rdup ( $it more ) }
    while
    rdrop
} addfn

/* map < list code -- [ ... ] > */
'map {
    rpush rpush [ rpop rpop foreach ]
} addfn

/* mapnested < list code -- [[result] [result] ..] >
/* like map, but each return value is in its own sub-list */
'mapnested {
    rpush rpush
    [
        rpop rpop
          0 names "[" search insert
            names "]" search append
        foreach 
    ]
} addfn

/* tellall < msg -- > send "msg" and $lf to all connections, and $out */
'tellall { $lf + {over sreplace} $connections $out append foreach } addfn

/* incomming < connection -- > Called by ChServer when a new connection has arrived.  */
'incomming { $connections swap append 'connections !  } addfn

/* connectionActivity    < connection -- [results] > 
 * called by a Connection when input is available.  This default
 * implementation treats the input as chaos code and evals it.  
 */
'connectionActivity { { slice " " join eval } } addfn

'globalGet { names swap element } addfn
'globalSet { names unrot insert } addfn

local 'symbols !

/* getSymbol < name -- symbol > */
'getSymbol {
    dup isSymbol dup
        {swap drop}
        {
            drop dup
            $symbols ( 0 swap ! )
            $symbols swap element
        }
    if
} addfn

/* 'Error < name -- error > */
'Error { getSymbol [ ] swap append 0 append "" append 'Error new } addfn

/* 'errorName < error -- name > */
'errorName { 0 element 1 element } addfn

'forget {names swap delete drop} addfn

/* listof < count element -- [ element element ... element ] > */
'listof {
    [] swap append
      { dup concat }
      { 2dup len > }
    while
    swap
    0 swap slice
} addfn

/* @ < name -- value > */
'@ {
    scopes swap element  /* will throw ~dictnf if name isn't defined */
    2 element
} addfn

/* time < -- seconds > */
'time {
    gettimeofday 0 element
} addfn

'finetime {
    gettimeofday splice 
      swap 'Real new 
      swap 'Real new 
    1000000 / +
} addfn

'timer {
    finetime rpush
    eval
    finetime rpop -
} addfn

"examples/numtostr.Ch" include

