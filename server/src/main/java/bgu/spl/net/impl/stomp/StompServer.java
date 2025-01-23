package bgu.spl.net.impl.stomp;
import java.io.BufferedOutputStream;
import java.io.OutputStream;

import bgu.spl.net.srv.Server;

public class StompServer{
    public static void main(String[] args) {
        final int NUMBER_OF_THREADS = 7;
         if(args[1].equals("tpc")){
            //open tpc instance
            System.out.println("pipi");
            Server.threadPerClient(Integer.parseInt(args[0]),
            () -> new StompProtocol(),
            () -> new StompEncoderDecoder()).serve();
        }

        else if(args[1].equals("reactor") ){
            //open reactor server
            System.out.println("kaka");
            Server.reactor(NUMBER_OF_THREADS,
            Integer.parseInt(args[0]),
            () -> new StompProtocol(),
            () -> new StompEncoderDecoder()).serve();
        }

        else{
            throw new IllegalArgumentException ("Invalid arguments");
        }
    }
}
