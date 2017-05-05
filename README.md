# parallel-alsa-modular
Parallelization of the ALSA Modular Synthesizer. http://alsamodular.sourceforge.net/
Due to commitments with Concordia University, Montreal, I cannot post the full working program. Instead I have decided to post some of the .h and .cpp files I used for parallelization.

Threadpool.cpp An implementation of a C++ Threadpool that processes Standard Futures

m_vca.cpp One of the ALSA Modulars modules, that shows how futures are handled and how branches in the Call Graph are generated.

thread_safe_queue.h a simple thread safe queue

Please Note# I have completed another version of this projest which uses a different parallelization this can be downloaded from the same github repository see: "ALSA-Modular-Parallel-Synth"

