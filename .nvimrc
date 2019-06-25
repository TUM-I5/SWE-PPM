" INDENTATION
set tabstop=8
set shiftwidth=8
set softtabstop=0
set noexpandtab

" LINTER FLAGS
let g:ale_cpp_gcc_options='-std=c++14 -Wall -O3 -DUPCXX_ASSERT_ENABLED=0 -DUPCXX_BACKEND=1 -DUPCXX_BACKEND_GASNET_SEQ=1 -DUPCXX_LPC_INBOX_lockfree=1 -D_GNU_SOURCE=1 -DGASNET_SEQ -I/home/jurek/storage/upcxx/gasnet.opt/include -I/home/jurek/storage/upcxx/gasnet.opt/include/smp-conduit -Isrc -I/home/jurek/storage/asagi/include -I/home/jurek/storage/upcxx/upcxx.O3.gasnet_seq.smp/include -L/home/jurek/storage/asagi/lib -L/home/jurek/storage/upcxx/upcxx.O3.gasnet_seq.smp/lib -lupcxx -L/home/jurek/storage/upcxx/gasnet.opt/lib -lgasnet-smp-seq -lrt -L/usr/lib/gcc/x86_64-pc-linux-gnu/8.1.0 -lgcc -lm -O3 --param max-inline-insns-single=35000 --param inline-unit-growth=10000 --param large-function-growth=200000 -Wno-unused -Wno-unused-parameter -Wno-address -DUPCXX -fopenmp'
"let g:ale_cpp_clang_options='-std=c++14 -Wall -O3 --param max-inline-insns-single=35000 --param inline-unit-growth=10000 --param large-function-growth=200000 -Wno-unused -Wno-unused-parameter -Wno-address -Isrc -L/home/jurek/storage/upcxx/upcxx.O3.gasnet_seq.smp/lib -lupcxx -L/home/jurek/storage/upcxx/gasnet.opt/lib -lgasnet-smp-seq -lrt -L/usr/lib/gcc/x86_64-pc-linux-gnu/7.3.1 -lgcc -lm -DUPCXX_ASSERT_ENABLED=0 -DUPCXX_BACKEND=1 -DUPCXX_BACKEND_GASNET_SEQ=1 -DUPCXX_LPC_INBOX_lockfree=1 -D_GNU_SOURCE=1 -DGASNET_SEQ -I/home/jurek/storage/upcxx/gasnet.opt/include -I/home/jurek/storage/upcxx/gasnet.opt/include/smp-conduit -I/home/jurek/storage/upcxx/upcxx.O3.gasnet_seq.smp/include'
