[ 
    "tests/00testsetup" 
    "tests/01critical"
    "tests/10intops"
    "tests/11stringops"
    "tests/12variables"
    "tests/13vectors"
    "tests/14file"
    "tests/15Set"
    "tests/21iteration"
    "tests/22scope"
    "tests/30returnstack"
    "tests/31parser"
    "tests/90misc"
] iterator 'it !

    { $it next include }
    { $it more }
while

"Tests done" emitln

/* cleaning up... */
this length drop 0
    { "Emptying junk left on stack by testing:" $lf + emit
      {.} {this len} while
      "Done" emitln
    }
    {}
if
