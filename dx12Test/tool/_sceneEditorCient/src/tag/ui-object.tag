const ClientServer = require('../js/client-server.js');
const vectorTag = require('./component-vector.js');
const Objects = require('../js/model-object.js');

<object-ui>
  <p>オブジェクト</p>
  <div>
    <label>name:</label>
    <input value={this.obj.name} oninput={changeName} type="text">
    <input if={!this.isExist} onclick={addObject} type="button" value="この名前のオブジェクトを追加">
    <input if={ this.isExist} onclick={deleteObject} type="button" value="この名前のオブジェクトを削除">
  </div>
  <vector name="pos" title="pos:" values={this.obj.orientation.pos} mixin_name={this.mixinName}></vector>
  <vector name="rot" title="rot:" values={this.obj.orientation.rot} mixin_name={this.mixinName}></vector>
  <vector name="scl" title="scale:" values={this.obj.orientation.scl} mixin_name={this.mixinName}></vector>
  <div>
    <label>model:</label>
    <input value={this.obj.model} oninput={updateValue} name="model" type="text">
  </div>
  <div>
    <label>render-phase:</label>
    <div each={this.obj.render_phase}>
      <input value={phaseName} oninput={updateRenderPhaseElement} name="render-phase-name" type="text">
      =
      <input value={pipelineName} oninput={updateRenderPhaseElement} name="pipeline-name"type="text">
      <input onclick={deleteRenderPhase} type="button" value="削除">
    </div>
    <input onclick={addRenderPhase} type="button" value="描画フェーズの追加">
  </div>
  <script>
    this.ieExist = false
    this.obj = {
      name: opts.name,
      orientation: {
        pos: opts.orientation.pos,
        rot: opts.orientation.rot,
        scl: opts.orientation.scl
      },
      model: opts.model,
      render_phase: opts.render_phase,
    }

    var Mixin = {
      init: function() {
        this.obs = riot.observable();
        this.obs.on('update', this.updateObject);
      },
      fireUpdate: function(key, value) {
        this.obs.trigger('update', key, value)
      },
      /**
       *  指定されたキーと一致するオブジェクトのメンバの値を更新する
       *  必ず、Mixin変数をriot.mixin関数に渡す前にすること
       */
      updateObject: function(key, value){
        console.log("Mixin.updateObject");
        console.log(key);
        console.log(value);
        //UIと実データは別のものとしているので、変更されるたびに実データも変更します
        //描画フェーズは別の関数で更新してます。
        var updateData = {};
        switch(key) {
          case 'pos':
            this.obj.orientation.pos = value.slice();
            updateData.orientation = {}
            updateData.orientation.pos = value.slice();
            break;
          case 'rot':
            this.obj.orientation.rot = value.slice();
            updateData.orientation = {}
            updateData.orientation.rot = value.slice();
            break;
          case 'scl':
            this.obj.orientation.scl = value.slice();
            updateData.orientation = {}
            updateData.orientation.scl = value.slice();
            break;
          case 'model':
            this.obj.model = value;
            updateData.model = value;
            break;
        }

        if(this.isExist) {
          Objects.update(this.obj.name, updateData);
        }
        var obj = Objects.find('test');
        console.log(obj);
      },
    }
    this.mixin(Mixin);
    Mixin.updateObject = Mixin.updateObject.bind(this);
    this.mixinName = 'object-common';
    riot.mixin(this.mixinName, Mixin);

    changeName(event) {
      console.log('ui-object: changeName')
      let name = event.target.value;
      let obj = Objects.find(name);
      this.isExist = (undefined != obj);
      if(this.isExist) {
        //存在するなら、それを表示する
        Objects.deepUpdate(this.obj, obj);
        this.update(this.obj);
      }else{
        this.obj.name = name
      }
    }
    updateValue(event) {
      console.log(event);
      this.fireUpdate(event.target.name, event.target.value);
    }
    addObject(event) {
      console.log('ui-object: addObject')
      let newObject = Objects.create();
      Objects.deepUpdate(newObject, this.obj);
      Objects.add(newObject)
      this.isExist = true;
    }
    deleteObject(event) {
      console.log('ui-object: deleteObject')
      Objects.delete(event.obj.name);
      this.isExist = false;
    }
    updateRenderPhaseElement(event) {
      console.log(event)
      for(var i=0; i<this.obj.render_phase.length; ++i) {
        let v = this.obj.render_phase[i];
        if( v.phaseName == event.item.phaseName ) {
          switch(event.target.name) {
            case 'render-phase-name': v.phaseName = event.target.value; break;
            case 'pipeline-name':     v.pipelineName = event.target.value; break;
          }
        }
      }

      if(this.isExist) {
        Objects.update(this.obj.name, {render_phase: this.obj.render_phase});
      }
    }
    addRenderPhase(event) {
      console.log('ui-object: addRenderPhase')
      this.obj.render_phase.push({phaseName: 'unknown', pipelineName:'unknown'});

      if(this.isExist) {
        Objects.update(this.obj.name, {render_phase: this.obj.render_phase});
      }
    }
    deleteRenderPhase(event) {
      console.log('ui-object: deleteRenderPhase')
      let targetPhase = event.item;
      for(var i=0; i<this.obj.render_phase.length; ++i) {
        let v = this.obj.render_phase[i];
        if(v.phaseName == targetPhase.phaseName) {
          this.obj.render_phase.splice(i, 1);
          break;
        }
      }

      if(this.isExist) {
        Objects.update(this.obj.name, {render_phase: this.obj.render_phase});
      }
    }

  </script>
</object-ui>
