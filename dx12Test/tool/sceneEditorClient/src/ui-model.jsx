const React = require('react');
const ReactDOM = require('react-dom');
const Models = require('./model-model.jsx');

const IDS = {
  name: 'model-name',
  name_list: 'model-name-list',
  add_object: 'model-add',
  delete_object: 'model-delete',
};

function createModel() {
  console.error('createModel関数 : 作ってね');
  let name = document.getElementById(IDS.name).value;
  return Models.createSystem(name, 'tetra', 'ptnc', Models.createMaterial([1, 1, 1], 1, [1, 1, 1], 10, {dif:'data/panda.dds'}));
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
    let obj = Models.find(event.target.value);
    if(obj) {
      this.setState({name: event.target.value});
      renderModel(obj);
    } else {
      this.setState({name: event.target.value});
    }
  }
  addObject(event) {
    let obj = createModel();
    Models.addAndSend(obj);
    renderModelDatalist();
    //HTMLの更新タイミングの関係なのか、ReactDOM.renderでオブジェクトを再描画するとdatalistが空になってしまう。
    //なので、代わりに状態を更新して、対応
    this.setState({name: this.state.name});
  }
  deleteObject(event) {
    Models.deleteAndSend(this.state.name);
    renderModelDatalist();
    //HTMLの更新タイミングの関係なのか、ReactDOM.renderでオブジェクトを再描画するとdatalistが空になってしまう。
    //なので、代わりに状態を更新して、対応
    this.setState({name: this.state.name});
  }
  render() {
    //実装上の都合でdatalistタグの直下にはdivタグがあるので、変な処理になってます。
    // let datalist = document.getElementById(IDS.name_list).children[0].children;
    // let isExist = ( undefined == Array.prototype.find.call(datalist, (element, index, array) => {
    //   return element.value == this.state.name
    // }) );
    let isExist = false;
    if(Models.find(this.state.name)){
      isExist = true;
    }

    let button = null;
    if(isExist) {
      button = (<input id={IDS.delete_object} onClick={this.deleteObject} type="button" value="この名前のモデルを削除" />);
    }else {
      button = (<input id={IDS.add_object} onClick={this.addObject} type="button" value="この名前のモデルを追加" />);
    }

    return (
      <div>
        name:<input id={IDS.name} type="text" list={IDS.name_list} value={this.state.name} onChange={this.changeValue}/>
        {button}
      </div>
    );
  }
}

class SystemPrimitive extends React.Component {
  constructor(props) {
    super(props);
    this.state = {value: props.prm};
    this.onChange = this.onChange.bind(this);
  }
  onChange(event) {
    this.setState({value: event.target.value});

    let updateData = {
      system: {
        prm: event.target.value
      }
    };
    let name = document.getElementById(IDS.name).value;
    Models.updateAndSend(name, updateData);
  }
  render() {
    return (
      <div>
        primitive:
        <select value={this.state.value} onChange={this.onChange}>
          <option value="plane">平面</option>
          <option value="tetra">三角錐</option>
        </select>
      </div>
    );
  }
}

class VertexType extends React.Component {
  constructor(props) {
    super(props);
    this.state = {value: props.type};
    this.onChange = this.onChange.bind(this);
  }
  onChange(event) {
    this.setState({value: event.target.value});

    let updateData = {
      system: {
        vertex: event.target.value
      }
    };
    let name = document.getElementById(IDS.name).value;
    Models.updateAndSend(name, updateData);
  }
  render() {
    return (
      <div>
        vertex:
        <select value={this.state.value} onChange={this.onChange}>
          <option value="p">P</option>
          <option value="pc">PC</option>
          <option value="pt">PT</option>
          <option value="pn">PN</option>
          <option value="ptn">PTN</option>
          <option value="ptnc">PTNC</option>
        </select>
      </div>
    );
  }
}

class Vector3 extends React.Component {
  constructor(props) {

  }
  render() {
    return (
      <div>
      </div>
    );
  }
}

class Material extends React.Component {
  constructor(props) {
    super(props);
  }
  render() {
    return (
      <div>
      </div>
    );
  }
}

class System extends React.Component {
  constructor(props) {
    super(props);
  }
  render() {
    return (
      <div>
        <SystemPrimitive prm={this.props.data.prm} />
        <VertexType type={this.props.data.vertex} />
        <Material data={this.props.data.mtl} />
      </div>
    );
  }
}

class Main extends React.Component {
  render() {
    return (
      <div>
        モデル
        <Name name={this.props.name} />
        <System data={this.props.system}/>
      </div>
    );
  }
};

function renderModel(data)
{
  //内部要素がステートフルな要素で大半が構成されて、Reactの更新がうまく働かないので、
  // 子要素を全て削除した上で、再描画するようにしている。
  let tag = document.getElementById('editor');
  for(var i=0; i<tag.children.length; ++i) {
    tag.children[i].remove();
  }
  ReactDOM.render(
    React.createElement(Main, data),
    tag
  );
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

function renderModelDatalist()
{
  ReactDOM.render(
    <Datalist list={Models.list}/>,
    document.getElementById(IDS.name_list)
  );
}

module.exports = {
  render: renderModel,
  renderDatalist: renderModelDatalist,
  component: Main,
  datalistComponent: Datalist,
  ids: IDS,
}
