import DS from 'ember-data';

export default DS.Model.extend({
  name: DS.attr('string'),
  createdBy: DS.belongsTo('user'),
  createdAt: DS.attr('date'),
  updatedAt: DS.attr('date'),
  awsKey: DS.attr('string'),
  awsRegion: DS.attr('string'),
  bits: DS.attr('string'),
  status: DS.attr('string'),
  progress: DS.attr('number'),
  type: DS.attr('string')
});
