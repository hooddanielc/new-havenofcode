import Ember from 'ember';

export default Ember.Component.extend({
  file: null,
  savingInitialPromise: false,
  queue: Ember.A(),
  store: Ember.inject.service('store'),

  createPromises: function (files) {
    const result = [];

    for (let i = 0; i < files.length; ++i) {
      result.push(this.get('store').createRecord('file', {
        name: files[i].name,
        bytes: files[i].size,
        type: files[i].type
      }));
    }

    console.log(result);

    return Ember.RSVP.Promise.all(result.map((file) => {
      return file.save();
    }));
  },

  actions: {
    upload: function () {
      const files = this.$('input[type="file"]')[0].files;

      this.createPromises(files).then((res) => {
        console.log('files', res);
      });
    }
  }
});
