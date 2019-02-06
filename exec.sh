cat test1.wlp4 | wlp4scan | wlp4parse | ./a.out > foo.asm
cs241.linkasm < foo.asm  > output.merl
cs241.linker output.merl print.merl alloc.merl >  final.mips
mips.array final.mips
