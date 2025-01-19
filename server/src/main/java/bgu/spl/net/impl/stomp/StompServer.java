package bgu.spl.net.impl.stomp;
import bgu.spl.net.srv.Server;

public class StompServer{
    public static void main(String[] args) {

        final int NUMBER_OF_THREADS = 7;
        if(args[1] == "tpc"){
            //open tpc instance
            
            Server.threadPerClient(Integer.parseInt(args[0]),
            () -> new StompMessagingProtocolImpl<Frame>(),
            () -> new StompMessageEncoderDecoder<Frame>()).serve();
        }

        else if(args[1] == "reactor"){
            //open reactor server
            Server.Reactor(NUMBER_OF_THREADS,
            Integer.parseInt(args[0]),
            () -> new StompMessagingProtocolImpl<Frame>(),
            () -> new StompMessageEncoderDecoder<Frame>()).serve();
        }

        else{
            throw new IllegalArgumentException ("Invalid arguments");
        }
    }
}
