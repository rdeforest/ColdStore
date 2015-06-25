/* attempt to render lists as strings */

/* listtostr < list -- str > */
'listtostr {
    dup
        {   rpush
            '[
                { rpop dup 0 delete rpush
                            0 element
                    datatostr append " " append
                }
                { rpop dup rpush }
            while
            rpop drop
            -2 delete
        }
        { drop "[]" } 
    if
} addfn
