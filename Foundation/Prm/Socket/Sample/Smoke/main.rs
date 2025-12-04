use prm_socket::*;
use std::thread;

fn main() {
    init().unwrap();
    let server = tcp_socket().unwrap();
    let port = bind_loopback(server, 0).unwrap();
    listen(server).unwrap();

    let handle = thread::spawn(move || {
        let acc = accept(server).unwrap();
        let mut buf = [0u8; 64];
        let n = recv(acc, &mut buf).unwrap();
        assert_eq!(&buf[..n], b"ping");
        close(acc);
        close(server);
    });

    let client = tcp_socket().unwrap();
    connect_loopback(client, port).unwrap();
    send(client, b"ping").unwrap();
    close(client);
    handle.join().unwrap();
    cleanup();
    println!("Socket OK");
}

