<vector>
  <div>
    <label>{opts.title}</label>
    <input each={value, index in this.elements} name={index} value={value} oninput={changeValue} type="number">
  </div>

  <script>
    this.name = opts.name;
    this.elements = opts.values;
    // this.getValue = function(){
    //   return this.elements;
    // }
    if( opts.mixin_name ) {
      console.log(`component-vector: set mixin! name=${opts.mixin_name}`);
      this.mixin(opts.mixin_name);
    }

    changeValue(event){
      console.log(`Vector changeValue name=${this.name}`)
      // console.log(event)
      // console.log(event.item) // <- event.itemのはループ展開された時点の値になっているので更新には使えない
      var v = parseFloat(event.target.value);
      if(isFinite(v)) {
        this.elements[event.item.index] = v;
      }
      this.fireUpdate(this.name, this.elements);
    }
  </script>
</vector>
