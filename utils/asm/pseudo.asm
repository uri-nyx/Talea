#once

#ruledef {
    seqz {rd: reg}, {rs: reg} => asm {
        sltiu {rd}, {rs}, 1
    }
    snez {rd: reg}, {rs: reg} => asm {
        sltu {rd}, zero, {rs}
    }
    beqz {rd: reg}, {label} => asm {
        beq {rd}, zero, {label}
    }
    bnez {rd: reg}, {label} => asm {
        bne {rd}, zero, {label}
    }
    li {rd: reg}, {const: i32} => {
        assert(const < 0x3fff)
        asm {
        addi {rd}, zero, {const}`15
        }
    }
    li {rd: reg}, {const: i32}  => asm {
        lui {rd}, ({const} >> 12)`20
        addi {rd}, {rd}, {const}`12
    }
    la {rd: reg}, {label} => asm {
        auipc    {rd}, (({label}-$) >> 12)`20
        addi     {rd},{rd},(({label}-($-4))`12)
    }
    llb {rd: reg}, {label} => asm {
        auipc {rd},(({label}l-$) >> 12)`20
        lb {rd},(({label}l-($-4))`12)({rd})

    }
    llh {rd: reg}, {label} => asm {
        auipc {rd},(({label}-$) >> 12)`20
        lh {rd},(({label}-($-4))`12)({rd})
    }
    llw {rd: reg}, {label} => asm {
        auipc {rd},(({label}-$) >> 12)`20
        lw {rd},(({label}-($-4))`12)({rd})
    }
    ssb{rd: reg}, {label}, {rt: reg} => asm {
        auipc    {rt},(({label}-$) >> 12)`20
        sb {rd},(({label}-($-4))`12)({rt})
    }
    ssh {rd: reg}, {label}, {rt: reg} => asm {
        auipc    {rt},(({label}-$) >> 12)`20
        sh {rd},(({label}-($-4))`12)({rt})
    }
    ssw {rd: reg}, {label}, {rt: reg} => asm {
        auipc    {rt},(({label}-$) >> 12)`20
        sw {rd},(({label}-($-4))`12)({rt})
    }
    call {label} => {
        offset = ({label}-$)`12
        assert((label % 4) == 0)
        ; CALLS SHOULD BE ALIGNED
        asm {
            auipc    ra, (({label}-$) >> 12)`20
            jalr     ra, ({offset})(ra)
        }
    }
    tail {label}, {rt: reg} => {
        offset = (label-$)`12
        assert((offset % 4) == 0)
        ; Tail Calls SHOULD BE ALIGNED
        asm {
            auipc    {rt}, (({label}-$) >> 12)`20
            jalr     zero, (offset) ({rt})
        }
    }

    call [{offset}] => {
        ; CALLS SHOULD BE ALIGNED
        asm {
            auipc    ra, ((offset) >> 12)`20
            jalr     ra, (offset`12) (ra)
        }
    }
    tail [{offset}], {rt: reg} => {
        ; Tail Calls SHOULD BE ALIGNED
        asm {
            auipc    {rt}, ((offset) >> 12)`20
            jalr     zero, (offset`12) ({rt})
        }
    }

    mv {rd: reg}, {rs: reg} => asm {addi {rd}, {rs}, 0}
    neg {rd: reg}, {rs: reg} => asm {sub {rd}, zero, {rs}}
    j {label} => asm {jal zero, {label}}
    j [{offset}] => asm {jal zero, [{offset}]}
    jr {rs: reg} => asm {jalr zero, 0({rs})}
    ret => asm {jalr zero, 0(ra)}
}