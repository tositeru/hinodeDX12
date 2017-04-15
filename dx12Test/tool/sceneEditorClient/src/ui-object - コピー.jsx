const React = require('react');
const ReactDOM = require('react-dom');
const Objects = require('./model-object.jsx');

const IDS = {
  name: 'object-name',
  name_list: 'object-name-list',
  pos_x: 'object-pos-x',
  pos_y: 'object-pos-y',
  pos_z: 'object-pos-z',
  rota_x: 'object-rot-x',
  rota_y: 'object-rot-y',
  rota_z: 'object-rot-z',
  rota_w: 'object-rot-w',
  scale_x: 'object-scale-x',
  scale_y: 'object-scale-y',
  scale_z: 'object-scale-z',
  model_key: 'object-model-key',
  model_key_list: 'object-model-key-list',
  render_phase_contents: 'object-render-phase-contents',
  render_phase_add: 'object-render-phase-add',
  pipeline_list: 'object-pipeline-list',
  render_phase_list: 'object-render-phase-list',
  add_object: 'object-add',
  delete_object: 'object-delete',
};

/**
 *
 */
function toVectorElement(out, key, value) {
  value = parseFloat(value);
  if(Number.isNaN(value)) {value = 0};
  switch(key) {
    case 'x': out[0] = value; break;
    case 'y': out[1] = value; break;
    case 'z': out[2] = value; break;
    case 'w': out[3] = value; break;
    default: console.error('未対応');
  }
}

/**
 *  数値が変更されたときの値チェック
 */
var validateChangeValue = function(event, defaultValue) {
  if(event.target.value == '') {
    if(defaultValue) {
      return defaultValue;
    }else{
      return '';
    }
  }else {
    return event.target.value;
  }
}

/**
 *  現在の入力内容からオブジェクトを作成する
 */
function createObject() {
  let name = document.getElementById(IDS.name).value;
  let pos = [
    document.getElementById(IDS.pos_x).valueAsNumber,
    document.getElementById(IDS.pos_y).valueAsNumber,
    document.getElementById(IDS.pos_z).valueAsNumber
  ];
  let rota = [
    document.getElementById(IDS.rota_x).valueAsNumber,
    document.getElementById(IDS.rota_y).valueAsNumber,
    document.getElementById(IDS.rota_z).valueAsNumber,
    document.getElementById(IDS.rota_w).valueAsNumber
  ];
  let scale = [
    document.getElementById(IDS.scale_x).valueAsNumber,
    document.getElementById(IDS.scale_y).valueAsNumber,
    document.getElementById(IDS.scale_z).valueAsNumber
  ];
  let modelKey = document.getElementById(IDS.model_key).value;
  let renderPhases = {};
  let renderPhaseTag = document.getElementById(IDS.render_phase_contents);
  for(let i=0; i<renderPhaseTag.children.length; ++i) {
    let element = renderPhaseTag.children[i];

    let key = null;
    let value = null;
    for(let j=0; j<element.children.length; ++j) {
      let tag = element.children[j];
      let name = tag.getAttribute('name');
      if( name == 'key') {
        key = tag.value;
      }else if(name == "value") {
        value = tag.value;
      }
    }

    if(key != null && value != null) {
      renderPhases[key] = value;
    }
  }
  return Objects.create(name, pos, rota, scale, modelKey, renderPhases);
}

function renderObject(obj)
{
  //内部要素がステートフルな要素で大半が構成されて、Reactの更新がうまく働かないので、
  // 子要素を全て削除した上で、再描画するようにしている。
  let tag = document.getElementById('editor');
  for(var i=0; i<tag.children.length; ++i) {
    tag.children[i].remove();
  }
  ReactDOM.render(
    React.createElement(Main, obj),
    tag
  );
}

function renderObjectNameList()
{
  ReactDOM.render(
    <Datalist list={Objects.list}/>,
    document.getElementById(IDS.name_list)
  );
}

class Name extends React.Component {
  constructor(props) {
    super(props);
    this.state = {name: props.name};
    this.changeValue = this.changeValue.bind(this);
    this.addObject = this.addObject.bind(this);
    this.deleteObject = this.deleteObject.bind(this);
  }
  changeValue(event) {
    let obj = Objects.find(event.target.value);
    if(obj) {
      this.setState({name: event.target.value});
      renderObject(obj);
    } else {
      this.setState({name: event.target.value});
    }
  }
  addObject(event) {
    let obj = createObject();
    Objects.addAndSend(obj);
    renderObjectNameList();
    //HTMLの更新タイミングの関係なのか、ReactDOM.renderでオブジェクトを再描画するとdatalistが空になってしまう。
    //なので、代わりに状態を更新して、対応
    this.setState({name: this.state.name});
  }
  deleteObject(event) {
    Objects.deleteAndSend(this.state.name);
    renderObjectNameList();
    //HTMLの更新タイミングの関係なのか、ReactDOM.renderでオブジェクトを再描画するとdatalistが空になってしまう。
    //なので、代わりに状態を更新して、対応
    this.setState({name: this.state.name});
  }
  render() {
    //実装上の都合でdatalistタグの直下にはdivタグがあるので、変な処理になってます。
    let datalist = document.getElementById(IDS.name_list).children[0].children;
    let isExist = ( undefined == Array.prototype.find.call(datalist, (element, index, array) => {
      return element.value == this.state.name
    }) );

    let button = null;
    if(isExist) {
      button = (<input id={IDS.add_object} onClick={this.addObject} type="button" value="この名前のオブジェクトを追加" />);
    }else {
      button = (<input id={IDS.delete_object} onClick={this.deleteObject} type="button" value="この名前のオブジェクトを削除" />);
    }

    return (
      <div>
        name:<input id={IDS.name} type="text" list={IDS.name_list} value={this.state.name} onChange={this.changeValue}/>
        {button}
      </div>
    );
  }
}

class Pos extends React.Component {
  constructor(props) {
    super(props);
    this.state = {x: props.value[0], y: props.value[1], z: props.value[2]};
    this.onChangeValue = this.onChangeValue.bind(this);
  }
  onChangeValue(event) {
    let val = validateChangeValue(event);
    this.setState({[event.target.name]: val});

    let updateData = {
      orientation: {
        pos: [this.state.x, this.state.y, this.state.z]
      }
    };
    toVectorElement(updateData.orientation.pos, event.target.name, val);
    let name = document.getElementById(IDS.name).value;
    Objects.updateAndSend(name, updateData);
  }
  render() {
    return (
      <div>
        pos:
        <input id={IDS.pos_x} name="x" type="number" value={this.state.x} onChange={this.onChangeValue}/>
        <input id={IDS.pos_y} name="y" type="number" value={this.state.y} onChange={this.onChangeValue}/>
        <input id={IDS.pos_z} name="z" type="number" value={this.state.z} onChange={this.onChangeValue}/>
      </div>
    );
  }
}

class Rota extends React.Component {
  constructor(props) {
    super(props);
    this.state = {x: props.value[0], y: props.value[1], z: props.value[2], w: props.value[3]};
    this.onChangeValue = this.onChangeValue.bind(this);
  }
  onChangeValue(event) {
    let val = validateChangeValue(event);
    this.setState({[event.target.name]: val});

    let updateData = {
      orientation: {
        rot: [this.state.x, this.state.y, this.state.z, this.state.w]
      }
    };
    toVectorElement(updateData.orientation.rot, event.target.name, val);
    let name = document.getElementById(IDS.name).value;
    Objects.updateAndSend(name, updateData);
  }
  render() {
    return (
      <div>
        rot:
        <input id={IDS.rota_x} name="x" type="number" value={this.state.x} onChange={this.onChangeValue}/>
        <input id={IDS.rota_y} name="y" type="number" value={this.state.y} onChange={this.onChangeValue}/>
        <input id={IDS.rota_z} name="z" type="number" value={this.state.z} onChange={this.onChangeValue}/>
        <input id={IDS.rota_w} name="w" type="number" value={this.state.w} onChange={this.onChangeValue}/>
      </div>
    );
  }
}

class Scale extends React.Component {
  constructor(props) {
    super(props);
    this.state = {x: props.value[0], y: props.value[1], z: props.value[2]};
    this.onChangeValue = this.onChangeValue.bind(this);
  }
  onChangeValue(event) {
    let val = validateChangeValue(event);
    this.setState({[event.target.name]: val});

    let updateData = {
      orientation: {
        scl: [this.state.x, this.state.y, this.state.z]
      }
    };
    toVectorElement(updateData.orientation.scl, event.target.name, val);
    let name = document.getElementById(IDS.name).value;
    Objects.updateAndSend(name, updateData);
  }
  render() {
    return (
      <div>
        scale:
        <input id={IDS.scale_x} name="x" type="number" value={this.state.x} onChange={this.onChangeValue}/>
        <input id={IDS.scale_y} name="y" type="number" value={this.state.y} onChange={this.onChangeValue}/>
        <input id={IDS.scale_z} name="z" type="number" value={this.state.z} onChange={this.onChangeValue}/>
      </div>
    );
  }
}

class ModelKey extends React.Component {
  constructor(props) {
    super(props);
    this.state = {value: props.value};
    this.onChangeValue = this.onChangeValue.bind(this);
  }
  onChangeValue(event) {
    let val = validateChangeValue(event, '');
    this.setState({[event.target.name]: val});

    let updateData = {
      model: val
    };
    let name = document.getElementById(IDS.name).value;
    Objects.updateAndSend(name, updateData);
  }
  render() {
    return (
      <div>
        model-key:
        <input id={IDS.model_key} name="value" type="text" list={IDS.model_key_list} value={this.state.value} onChange={this.onChangeValue}/>
      </div>
    );
  }
}

class RenderPhaseElement extends React.Component {
  constructor(props) {
    super(props);
    this.state = {key: props.value.key, value: props.value.value};
    this.prevProps = props;
    this.onChangeValue = this.onChangeValue.bind(this);
  }
  onChangeValue(event) {
    let val = validateChangeValue(event, '__unknown');
    this.setState({[event.target.name]: val});

    let key = this.state.key;
    let value = this.state.value;
    if(event.target.name == 'key') {
      key = val;
    } else {
      value = val;
    }

    let updateData = {
      render_phase: {
        [key]: value
      }
    };
    let name = document.getElementById(IDS.name).value;
    Objects.updateAndSend(name, updateData);
  }
  render() {
    //propsが変更されたら、stateの方に反映させる
    if(this.prevProps != this.props) {
      this.prevProps = this.props;
      this.setState({key: this.props.value.key, value: this.props.value.value});
    }

    return (
      <div>
        <input name="key" type="text" list={IDS.render_phase_list} value={this.state.key} onChange={this.onChangeValue}/>
        =
        <input name="value" type="text" list={IDS.pipeline_list} value={this.state.value} onChange={this.onChangeValue} />
        <input type="button" value="削除" name={this.state.key} onClick={this.props.value.onDelete}/>
      </div>
    );
  }
}

class RenderPhase extends React.Component {
  constructor(props) {
    super(props);
    this.state = {contents: props.contents};
    this.changeValue = this.changeValue.bind(this);
    this.addElement = this.addElement.bind(this);
    this.deleteElement = this.deleteElement.bind(this);
  }
  changeValue(event) {
    this.setState({[event.target.name]: event.target.value});
  }
  addElement(event) {
    let new_state = this.state;
    new_state.contents['__unknown'] = '__unknown';
    this.setState(new_state);

    let updateData = {
      render_phase: {__unknown: '__unknown'}
    };
    let name = document.getElementById(IDS.name).value;
    Objects.updateAndSend(name, updateData);
  }
  deleteElement(event) {
    let new_state = this.state;
    delete new_state.contents[event.target.name];
    this.setState(new_state);

    let updateData = {
      render_phase: {}
    };
    for(let key in new_state) {
      updateData.render_phase[key] = new_state[key];
    }
    let name = document.getElementById(IDS.name).value;
    Objects.updateAndSend(name, updateData);
  }
  render() {
    const contents = Object.keys(this.state.contents).map((key_value) => {
      let value = {key: key_value, value: this.state.contents[key_value], onDelete: this.deleteElement};
      return <RenderPhaseElement value={value} />;
    });
    return (
      <div>
        render-phase:
        <div id={IDS.render_phase_contents}>
          {contents}
        </div>
        <input id={IDS.render_phase_add} type="button" value="描画フェーズの追加" onClick={this.addElement}/>
      </div>
    );
  }
}

class Main extends React.Component {
  render() {
    return (
      <div>
        オブジェクト
        <Name name={this.props.name}/>
        <Pos value={this.props.orientation.pos}/>
        <Rota value={this.props.orientation.rot}/>
        <Scale value={this.props.orientation.scl}/>
        <ModelKey value={this.props.model}/>
        <RenderPhase contents={this.props.render_phase}/>
      </div>
    );
  }
}

/**
 *  存在しているオブジェクトの名前を集めたdatalistタグの中身を生成する
 */
class Datalist extends React.Component {
  render() {
    let items = this.props.list.map((item) => {
      return (<option value={item.name} />);
    });
    return (<div>{items}</div>);
  }
}

module.exports = {
  render: renderObject,
  renderDatalist: renderObjectNameList,
  component: Main,
  datalistComponent: Datalist,
  ids: IDS,
};
