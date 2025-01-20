package bgu.spl.net.impl.stomp;
import bgu.spl.net.srv.Server;
import bgu.spl.net.srv.StompFrame;

public class StompServer{
    public static void main(String[] args) {

        // final int NUMBER_OF_THREADS = 7;
        // if(args[1] == "tpc"){
        //     //open tpc instance
            
        //     Server.threadPerClient(Integer.parseInt(args[0]),
        //     () -> new StompMessagingProtocolImpl<Frame>(),
        //     () -> new StompMessageEncoderDecoder<Frame>()).serve();
        // }

        // else if(args[1] == "reactor"){
        //     //open reactor server
        //     Server.Reactor(NUMBER_OF_THREADS,
        //     Integer.parseInt(args[0]),
        //     () -> new StompMessagingProtocolImpl<Frame>(),
        //     () -> new StompMessageEncoderDecoder<Frame>()).serve();
        // }

        // else{
        //     throw new IllegalArgumentException ("Invalid arguments");
        // }

        //String frame = "CONNECT\nfirst:header1\nsecond:header2\n\nthis is a message body\nasdf;lkj";
        StringBuilder builder = new StringBuilder();
        builder.append("CONNEC\nfirst:header1\nsecond:header2\n\nthis is a message body\nasdf;lkj");
        System.out.println(builder.toString());


        //StompFrame fram = new StompFrame(frame.getBytes());

        //System.out.println(fram.getHeaderMap());
    }
}
