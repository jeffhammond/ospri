#include <exception>

#include <dcmf.h>

namespace OSP
{
    class Get
    {
        public:
            Get()
            {
                conf.protocol = DCMF_DEFAULT_GET_PROTOCOL;
                conf.network  = DCMF_TORUS_NETWORK;

                int status = DCMF_Get_register( &proto, &conf );
                if ( status != DCMF_SUCCESS) throw status;
            };

            ~Get()
            {

            };

        private:
            DCMF_Get_Configuration_t conf;
            DCMF_Protocol_t proto;
    };
}
