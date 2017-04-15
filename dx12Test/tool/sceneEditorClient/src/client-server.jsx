/**
 *  クライアントサーバー
 */
class ClientServer {
  constructor() {
    console.log('ClientServer.constructor');
    this._websocket = null;
    this._openHandle = function(websocket, event) { }
    this._closeHandle = function(websocket, event) { }
    this._errorHandle = function(websocket, event) { }
    this._messageHandle = function(websocket, event) { }
  }

  /**
   *  サーバーとの接続を開始する
   */
  open(uri)
  {
    if(this._websocket) {
      this._websocket = null;
    }
    console.log("websocket.open : " + uri);
    this._websocket = new WebSocket(uri);

    let This = this;
    this._websocket.onopen = function(event) {
      console.log("onopen");
      This._openHandle(This._websocket, event);
    };
    this._websocket.onclose = function(event) {
      console.log("onclose");
      This._closeHandle(This._websocket, event);
      if(This._websocket) { This._websocket = null; }
    };
    this._websocket.onerror = function(event) {
      console.log("onerror");
      This._errorHandle(This._websocket, event);
    };
    this._websocket.onmessage = function(event) {
      console.log("onmessage");
      This._messageHandle(This._websocket, event);
    };
  }

  /**
   *  サーバーとの接続を閉じる
   */
  close()
  {
    if(null != this._websocket) {
      this._websocket.close();
      this._websocket = null;
    }
  }

  /**
   *  サーバーへメッセージを送る
   *  後々、バイナリも遅れるようにする？
   */
  send(msg)
  {
    console.log(msg);
    if(null == this._websocket) {
      console.error("ClientServer.send: don't connect to server...");
      return ;
    }
    this._websocket.send(msg);
  }
}

module.exports = new ClientServer;
