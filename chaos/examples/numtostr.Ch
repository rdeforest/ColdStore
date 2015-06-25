/* words for converting numbers to strings */

/* renderinteger < int base -- str > */
'renderinteger {
    2dup <
      { drop renderdigit }
      { 2dup % renderdigit rpush
        swap over /
        swap renderinteger
        rpop append
        }
    if
  } addfn

/* renderdigit < digit -- str > */
'renderdigit {
     "0123456789abcdefghijklmnopqrstuvwxyz" swap element
} addfn

'tohex { 16 renderinteger } addfn
'tooct {  8 renderinteger } addfn
'tobin {  2 renderinteger } addfn
'todec { 10 renderinteger } addfn

/* renderreal < real base scale -- str > 
 * 
 * Render real in 'base' digits with 'scale' digits after the decimal
 * point.
 */

'renderreal {
    local dup rpush (
        'scale !
        'base !

        $base $scale pow


    )
} addfn

/* pow < base exp -- base^exp > 
 * Uses the bone-head method for now, faster version later
 */
'pow {
    local (
      'exp !
      'base !
      1 'BigInt new
        { $base * $exp 1 - 'exp ! }
        { $exp }
      while
    )
} addfn

'floatpow {
    local (
      'exp !
      'base !
      1 'Real new
        { $base * $exp 1 - 'exp ! }
        { $exp }
      while
    )
} addfn

/* fastpow < base exp -- base^exp > 
 * n^2 = n*n
 * n^3 = (n*n) * m
 * n^4 = ((n * n) * (n * n))
 * n^5 = ((n * n) * (n * n)) * n
 * exp(n,2m) = exp(n,m)**2
 * exp(n,m+1) = exp(n,m)*n
 * */
'fastpow {
  dup 3 <
    { pow }
    { 
      dup 2 %
        { 1 - over rpush fastpow rpop * }
        { swap dup * swap 1 >> fastpow }
      if
    }
  if
} addfn

