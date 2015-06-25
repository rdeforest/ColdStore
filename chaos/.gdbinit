### remove the old store - good sometimes :)
shell rm ./coldstore

### set some breakpoints
break main
#break bkpt
break onError
break ColdTerminate
#break __assert_fail

### customise behavior
#set stop-on-solib-events 1
#set print static-members off
set print static-members on
set print vtbl on
set print object on
set print pretty on

### set up some environment
#set environment LD_LIBRARY_PATH=/usr/lib/libc_debug
#set environment LD_DEBUG all
set environment LD_LIBRARY_PATH=../libs/:.
directory ../qvmm/ ../intern/ ../coldstore/ ../chaos

### reRun starts the thing again with a clean store
define reRun
shell rm ./coldstore
#break onError
run
end

### Type gives you the type of a Slot
define Type
p $arg0.data_->typeId()
end

define graphit
graph display *($arg1*)($arg0.data_)
end
