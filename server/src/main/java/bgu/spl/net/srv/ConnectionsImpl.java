package bgu.spl.net.srv;

import java.util.Iterator;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import bgu.spl.net.api.MessageTransformer;
import bgu.spl.net.impl.stomp.StompFrame;

public class ConnectionsImpl<T> implements Connections<T>{

    private static ConnectionsImpl<?> instance;
    private ConcurrentHashMap<Integer,ConnectionHandler<T>> connectionsMap; // ID to Connection Handler
    private ConcurrentHashMap<String, ConcurrentLinkedQueue<Integer[]>> channelsMap; // Channel name to List of ID's
    private ConcurrentHashMap<Integer, String> activeUsers; // ID to usernames
    private ConcurrentHashMap<String, String> loginMap; // username to password
    private final ReentrantReadWriteLock lock;
                                         
    public static <T> ConnectionsImpl<T> getInstance(){
        if (instance == null){
            synchronized(ConnectionsImpl.class){
                if (instance == null){
                    instance = new ConnectionsImpl<>();
                }
            }
        }

        return (ConnectionsImpl<T>) instance;
    }

    private ConnectionsImpl(){
        this.connectionsMap = new ConcurrentHashMap<>();
        this.channelsMap = new ConcurrentHashMap<>();
        this.loginMap = new ConcurrentHashMap<>();
        this.activeUsers = new ConcurrentHashMap<>();
        this.lock = new ReentrantReadWriteLock();
    }


    // Send the message to the connection with the specific connection ID
    public boolean send(int connectionId, T message){
        if(connectionsMap.get(connectionId) != null){
            lock.readLock().lock();
            try{
                ConnectionHandler<T> handler = connectionsMap.get(connectionId);
                if(handler != null){
                    handler.send(message);
                    return true;
                }
            }
            finally{
                lock.readLock().unlock();       
            }
            
        }
        return false;
    }

    // Send the message for each active connection that subscribed to this specific channel
    public int send(String channel, T message, MessageTransformer<T> transformer, int connectionID){
        lock.readLock().lock();
        try{
            // Check if Client not subscribed to the channel
            if(!checkIfSubscribed(channel, connectionID)){
                System.out.println("KAKA ANAK");
                return -1;
            }

            else{
                ConcurrentLinkedQueue<Integer[]> subscribers = channelsMap.get(channel);
                if(subscribers != null){
                    for(Integer[] ID : channelsMap.get(channel)){
                        ConnectionHandler<T> handler = connectionsMap.get(ID[0]);
                        if (handler != null){
                            // Use transfromer to make protocol flexible
                            T newMessage = transformer.transformMessage(message, ID[1]);
                            System.out.println("message sent to " + activeUsers.get(connectionID));
                            handler.send(newMessage);
                        }
                    } 
                }

                return 1;
            }
        } finally{
            lock.readLock().unlock();  
        }
    }
    
    public void connect(int connectionId, ConnectionHandler<T> handler){
        lock.writeLock().lock();
        connectionsMap.put(connectionId, handler);
        lock.writeLock().unlock();       
    }

    // Remove an active connection from the connectionMap
    public ConnectionHandler<T> disconnect(int connectionId){
        unsubscribeChannel(connectionId);
        synchronized(activeUsers){
            activeUsers.remove(connectionId);
        }

        lock.writeLock().lock();
        ConnectionHandler<T> handler = connectionsMap.get(connectionId);
        connectionsMap.remove(connectionId);
        lock.writeLock().unlock();
        return handler;
    } 

    private void unsubscribeChannel(int connectionId){
        lock.writeLock().lock();
        try{
            for (String channel : channelsMap.keySet()){
                Iterator<Integer[]> iter = channelsMap.get(channel).iterator();
                while(iter.hasNext()){
                    Integer[] pair = iter.next();
                    if(pair[0] == connectionId){
                        iter.remove();
                    }
                }
            }
        } finally{
            lock.writeLock().unlock();
        }
    }

    public int login(int connectionId, String userName, String password){
        synchronized(loginMap){
            // user doesn't exist
            if(loginMap.get(userName) == null){ 
                loginMap.put(userName, password);          
            }
            
            synchronized(activeUsers){
                boolean userLoggedIn = false;
                for(String user : activeUsers.values()){
                    if(user.equals(userName)){
                        userLoggedIn = true;
                        break;
                    }
                }
                
                if(userLoggedIn){
                    return -2; // User already logged in =  -2
                }
            }

            // check if user password is correct
            if(loginMap.get(userName).equals(password)){       
                activeUsers.put(connectionId, userName);   
                return 1; // Logged in successfully
            }
    
            else{   
                return -1; // wrong password  = -1
            }
        }
    }

    public int subscribe(String channel, Integer subscriptionId, Integer connectionId){
        lock.writeLock().lock();
        try {
            channelsMap.computeIfAbsent(channel, k -> new ConcurrentLinkedQueue<>());
            ConcurrentLinkedQueue<Integer[]> currentChannel = channelsMap.get(channel);
            for (Integer[] keys : currentChannel){
                if (keys[0] == connectionId){
                    return -1;
                }
            }

            currentChannel.add(new Integer[]{connectionId,subscriptionId});
            return 1;

        } finally {
            for (String c : channelsMap.keySet()){
                System.out.print(c + ": ");
                for (Integer[] ids : channelsMap.get(c)){
                    System.out.print("("+ ids[0] + " " + ids[1] + ")" + ", ");
                }

                System.out.println("");
            }
            lock.writeLock().unlock();
        }        
    }

    public void unsubscribe(int connectionId, int subscriptionID) {
        lock.writeLock().lock();
        try{
            Iterator<ConcurrentLinkedQueue<Integer[]>> channelIter = channelsMap.values().iterator();
            while(channelIter.hasNext()){
                ConcurrentLinkedQueue<Integer[]> channel = channelIter.next();
                Iterator<Integer[]> subscribersIter = channel.iterator();
                while(subscribersIter.hasNext()){
                    Integer[] subscriber = subscribersIter.next();
                    if(subscriber[0] == connectionId && subscriber[1] == subscriptionID){
                        subscribersIter.remove();
                        return;
                    }

                } 
            }
        } finally {
            System.out.println(channelsMap);
            lock.writeLock().unlock();
        }        
    }

    private boolean checkIfSubscribed(String channel, int connectionId){
        System.err.println("ConnectionID :" + connectionId + " , Channel :" + channel);
        ConcurrentLinkedQueue<Integer[]> subscribers = channelsMap.get(channel);
        if(subscribers != null){
            for(Integer[] ID : subscribers){
                if(ID[0] == connectionId){
                    return true;
                }
            }
        }
        return false;
    }
}

