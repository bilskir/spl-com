package bgu.spl.net.impl.stomp;
import java.io.BufferedOutputStream;
import java.io.OutputStream;

import bgu.spl.net.srv.Server;

public class StompServer{
    public static void main(String[] args) {
         if(args[1].equals("tpc")){
            //open tpc instance
            System.out.println("tpc");
            Server.threadPerClient(Integer.parseInt(args[0]),
            () -> new StompProtocol(),
            () -> new StompEncoderDecoder()).serve();
        }

        else if(args[1].equals("reactor") ){
            //open reactor server
            System.out.println("reactor");
            Server.reactor(Runtime. getRuntime().availableProcessors(),
            Integer.parseInt(args[0]),
            () -> new StompProtocol(),
            () -> new StompEncoderDecoder()).serve();
        }

        else{
            throw new IllegalArgumentException ("Invalid arguments");
        }
    }
}
