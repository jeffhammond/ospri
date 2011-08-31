#include <exception>

#include <dcmf.h>

namespace OSP
{
    void generic_done(void *clientdata, DCMF_Error_t *error)
    {
        --( *( (int *) clientdata ) );
    }


    class Get
    {
        public:
            Get()
            {
                consistency = DCMF_RELAXED_CONSISTENCY;

                configuration.protocol = DCMF_DEFAULT_GET_PROTOCOL;
                configuration.network  = DCMF_TORUS_NETWORK;

                int status = DCMF_Get_register( &protocol, &configuration );
                if ( status != DCMF_SUCCESS) throw status;
            }

            ~Get()
            {

            }

            Start(int target, int bytes)
            {
                DCMF_Request_t request;

                callback.function = generic_done;

                volatile int active;
                callback.clientdata = (void *) &active;

                src_disp = (size_t) src - (size_t) OSPD_Membase_global[target];
                dst_disp = (size_t) dst - (size_t) OSPD_Membase_global[OSPD_Process_info.my_rank];

                active = 1;

                int status = DCMF_Get(&protocol,
                                      &request,
                                      callback,
                                      consistency,
                                      remote_rank,
                                      bytes,
                                      remote_memregion,
                                      local_memregion,
                                      remote_offset,
                                      local_offset);
                if ( status != DCMF_SUCCESS) throw status;

                while (active > 0) DCMF_Messager_advance();
            }

        private:
            DCMF_Get_Configuration_t configuration;
            DCMF_Protocol_t protocol;
            DCMF_Callback_t callback;
            DCMF_Consistency consistency;
    };
}
