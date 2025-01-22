package bgu.spl.net.impl.stomp;
import java.io.BufferedOutputStream;
import java.io.OutputStream;

import bgu.spl.net.srv.Server;

public class StompServer{
    public static void main(String[] args) {

        final int NUMBER_OF_THREADS = 7;
         if(args[1] == "tpc"){
            //open tpc instance
            
            Server.threadPerClient(Integer.parseInt(args[0]),
            () -> new StompProtocol(),
            () -> new StompEncoderDecoder()).serve();
        }

        else if(args[1] == "reactor"){
            //open reactor server
            Server.reactor(NUMBER_OF_THREADS,
            Integer.parseInt(args[0]),
            () -> new StompProtocol(),
            () -> new StompEncoderDecoder()).serve();
         }

        // else{
        //     throw new IllegalArgumentException ("Invalid arguments");
        // }

        //String frame = "CONNECT\nfirst:header1\nsecond:header2\n\nthis is a message body\nasdf;lkj";
        
        
    //     StringBuilder builder1 = new StringBuilder();
    //     builder1.append("CONNECT\nfirst:header1\nsecond:header2\n\nthis is a message body\nasdf;lkj\u0000");
    //     builder1.append("SUBSCRIBE\nfirst:header\nsecond:header\nthird:header\n\nsecond message nadi body\nla bla bla\nbla\u0000");
    //    // System.out.println(builder.toString());

        


    //     StompEncoderDecoder sed = new StompEncoderDecoder();

    //     byte[] bytes = builder1.toString().getBytes();
    //     StompFrame frame = null;
    //     for (byte b : bytes){
    //         frame = sed.decodeNextByte(b);
    //         // System.out.println(frame);
    //     }

    //     StompFrame newFrame = new StompFrame("ERROR","server couldnt connect");
    //     newFrame.addHeader("connectionId", "2");
    //     newFrame.addHeader("channel", "firestarter");


    //     System.out.println(new String(sed.encode(newFrame)));

        //StompFrame fram = new StompFrame(frame.getBytes());
        //System.out.println(fram.getHeaderMap());
    }
}
