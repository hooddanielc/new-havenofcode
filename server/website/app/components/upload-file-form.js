import Ember from 'ember';

export default Ember.Component.extend({
  file: null,
  savingInitialPromise: false,
  queue: Ember.A(),
  store: Ember.inject.service('store'),

  selectedFiles: function () {
    const files = this.$('input[type="file"]')[0].files;
    const result = [];

    for (let i = 0; i < files.length; ++i) {
      result.push(this.get('store').createRecord('file', {
        name: files[i].name,
        bits: files[i].size * 8,
        type: files[i].type
      }));
    }

    return result;
  }.property('file'),

  actions: {
    upload: function () {
      this.set('savingInitialPromise', true);
      const promises = [];

      this.get('selectedFiles').forEach((record) => {
        console.log(record.get('name'));
        console.log(record.get('bits'));
        console.log(record.get('type'));
        this.get('queue').pushObject(record);
        promises.push(record.save());
      });

      return Ember.RSVP.Promise.all(promises).then(() => {
        this.set('savingInitialPromise', false);
      }).catch((err) => {
        this.set('savingInitialPromise', false);
        console.log(err);
      });
    }
  }
});
