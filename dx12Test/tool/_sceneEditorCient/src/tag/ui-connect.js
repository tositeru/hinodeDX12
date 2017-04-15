
(function(tagger) {
  if (typeof define === 'function' && define.amd) {
    define(function(require, exports, module) { tagger(require('riot'), require, exports, module)})
  } else if (typeof module !== 'undefined' && typeof module.exports !== 'undefined') {
    tagger(require('riot'), require, exports, module)
  } else {
    tagger(window.riot)
  }
})(function(riot, require, exports, module) {
const ClientServer = require('../js/client-server.js');

riot.tag2('connect-ui', '<div> URI:ws:// <input ref="uri" type="text" riot-value="{this.uri}"> </div> <input if="{!this.isConnect}" onclick="{openConnect}" type="button" value="接続開始"> <input if="{this.isConnect}" onclick="{closeConnect}" type="button" value="接続終了"> <div> <label>接続状況</label> <div class="connect-info"> <p each="{msg in messages}">{msg}</p> </div> </div>', 'connect-ui .connect-info,[data-is="connect-ui"] .connect-info{ overflow-y: scroll; height: 150px; font-size: 12px; border: solid 1px #a3a3a3; }', '', function(opts) {
    this.uri = 'localhost:9002'
    this.isConnect = false
    this.messages = []

    this.appendMessage = function(msg){
      console.log(msg)
      this.messages.push(msg);
      this.update()
    }

    this.openConnect = function(event) {
        this.isConnect = true;
        this.uri = this.refs.uri.value
        console.log(this.uri);
        ClientServer.open('ws://' + this.uri);
    }.bind(this)
    this.closeConnect = function(event) {
        this.isConnect = false;
        ClientServer.close();
    }.bind(this)
});
});