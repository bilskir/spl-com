package bgu.spl.net.srv;

import java.util.Iterator;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import bgu.spl.net.impl.stomp.StompFrame;

public class ConnectionsImpl<T> implements Connections<T>{

    private static ConnectionsImpl<?> instance;
    private ConcurrentHashMap<Integer,ConnectionHandler<T>> connectionsMap;             // ID to Connection Handler
    private ConcurrentHashMap<String, ConcurrentLinkedQueue<Integer[]>> channelsMap;    // Channel name to List of ID's
    private ConcurrentHashMap<Integer, String> activeUsers;
    private ConcurrentHashMap<String, String> loginMap;                                 // username to password
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
                if(connectionsMap.get(connectionId) != null){
                    connectionsMap.get(connectionId).send(message);
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
    public void send(String channel, T message){
        lock.readLock().lock();
        try{
            for(Integer[] ID : channelsMap.get(channel)){
                if (connectionsMap.get(ID[0]) != null){
                    connectionsMap.get(ID[0]).send(message);
                }
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


        System.out.println(connectionsMap);
        System.out.println(activeUsers);

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
                    System.out.println(connectionsMap);
                    System.out.println(activeUsers);
                    return -2; // User already logged in =  -2
                }
            }

            // check if user password is correct
            if(loginMap.get(userName).equals(password)){       
                activeUsers.put(connectionId, userName);
                System.out.println(connectionsMap);
                System.out.println(activeUsers);
                return 1; // Logged in successfully
            }
    
            else{   
                System.out.println(connectionsMap);
                System.out.println(activeUsers);
                return -1; // wrong password  = -1
            }
        }
    }

    public void logout(int connectionId){
        unsubscribeChannel(connectionId);
        synchronized(activeUsers){
            activeUsers.remove(connectionId);
        }
    }
}

