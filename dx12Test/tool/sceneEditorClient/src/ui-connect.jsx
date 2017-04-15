var React = require('react');
var ClientServer = require('./client-server.jsx');

ID_URI = 'connect-uri';

class Uri extends React.Component {
  constructor(props) {
    super(props);
    this.state = {uri: 'localhost:9002'};
    this.handleChange = this.handleChange.bind(this);
  }
  handleChange(event) {
    this.setState({uri: event.target.value});
  }
  render() {
    return (
      <div>
        URI: ws://
        <input id={ID_URI} type="text" value={this.state.uri} onChange={this.handleChange} />
      </div>
    );
  }
}

class Open extends React.Component {
  render() {
    return (
      <input id="open-websocket" type="button" value="接続開始" onClick={this.props.onClick} />
    )
  }
}

class Close extends React.Component {
  render() {
    return (
      <input id="close-websocket" type="button" value="接続終了" onClick={this.props.onClick}/>
    )
  }
}

class Main extends React.Component {
  constructor(props) {
    super(props);
    this.state = {isConnect: false};
    this.onOpen = this.onOpen.bind(this);
    this.onClose = this.onClose.bind(this);
  }
  onOpen(event) {
    var uriTag = document.getElementById(ID_URI);
    console.log(uriTag);
    ClientServer.open("ws://" + uriTag.value);
    this.setState({isConnect: true});
  }
  onClose(event) {
    ClientServer.close();
    this.setState({isConnect: false});
  }
  render() {
    let button = null;
    if(this.state.isConnect) {
      button = <Close onClick={this.onClose} />;
    }else{
      button = <Open  onClick={this.onOpen} />;
    }

    return (
      <div>
        <Uri />
        {button}
      </div>
    );
  }
}

module.exports = {
  component: Main,
  ids: {
    uri: ID_URI,
  }
};
