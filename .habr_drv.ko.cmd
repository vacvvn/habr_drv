cmd_/home/vacvvn/software/examples/kernel_example/habr_drv/habr_drv.ko := ld -r -m elf_x86_64 -z max-page-size=0x200000 -T ./scripts/module-common.lds --build-id  -o /home/vacvvn/software/examples/kernel_example/habr_drv/habr_drv.ko /home/vacvvn/software/examples/kernel_example/habr_drv/habr_drv.o /home/vacvvn/software/examples/kernel_example/habr_drv/habr_drv.mod.o ;  true