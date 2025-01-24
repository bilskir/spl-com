package bgu.spl.net.impl.stomp;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;

import bgu.spl.net.api.MessageEncoderDecoder;

public class StompEncoderDecoder implements MessageEncoderDecoder<StompFrame> {
    private byte[] bytes = new byte[1 << 10];
    int len = 0;

    public StompEncoderDecoder() {
    }

    public StompFrame decodeNextByte(byte nextByte) {
        // System.out.println((char)nextByte);
        if (nextByte == '\u0000') {
            return makeFrame();
        }

        pushByte(nextByte);
        return null;
    }

    public byte[] encode(StompFrame message) {
        StringBuilder builder = new StringBuilder();
        builder.append(message.getCommand() + "\n");
        for (String key : message.getHeaderMap().keySet()) {
            builder.append(key + ":" + message.getHeaderMap().get(key) + "\n");
        }
        builder.append("\n" + message.getBody() + "\n");
        builder.append("\u0000");
        return builder.toString().getBytes();
    }

    private void pushByte(byte nextByte) {
        if (len >= bytes.length) {
            bytes = Arrays.copyOf(bytes, len * 2);
        }

        bytes[len] = nextByte;
        len++;
    }

    private StompFrame makeFrame() {
        String StompMessage = new String(bytes, 0 ,len  , StandardCharsets.UTF_8);
        
        // System.out.println("--- TEST MESSAGE --- \n" + StompMessage + "\n --- END TEST --- \n");
        String[] arr = StompMessage.split("\n\n");
        String[] content = arr[0].split("\n");
        String command = content[0];
        String body = "";
        
        if(arr.length > 1){
            body = arr[1];
        }

        StompFrame frame = new StompFrame(command, body);

        for (int i = 1; i < content.length; i++) {
            String key = content[i].split(":")[0];
            String val = content[i].split(":")[1];
            frame.addHeader(key, val);
        }

       
        len = 0;
        return frame;
    }

}
