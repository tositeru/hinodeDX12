const ClientServer = require('../js/client-server.js');

<connect-ui>
  <div>
    URI:ws://
    <input ref="uri" type="text" value={this.uri}>
  </div>
  <input if={!this.isConnect} onclick={openConnect} type="button" value="接続開始">
  <input if={ this.isConnect} onclick={closeConnect} type="button" value="接続終了">
  <div>
    <label>接続状況</label>
    <div class="connect-info">
    <p each={msg in messages}>{ msg }</p>
    </div>
  </div>
  <style>
    .connect-info {
      overflow-y: scroll;
      height: 150px;
      font-size: 12px;
      border: solid 1px #a3a3a3;
    }
  </style>
  <script>
    this.uri = 'localhost:9002'
    this.isConnect = false
    this.messages = []

    this.appendMessage = function(msg){
      console.log(msg)
      this.messages.push(msg);
      this.update()
    }

    openConnect(event) {
        this.isConnect = true;
        this.uri = this.refs.uri.value
        console.log(this.uri);
        ClientServer.open('ws://' + this.uri);
    }
    closeConnect(event) {
        this.isConnect = false;
        ClientServer.close();
    }
  </script>
</connect-ui>
