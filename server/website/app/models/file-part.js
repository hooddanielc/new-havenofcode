import DS from 'ember-data';

export default DS.Model.extend({
  name: DS.attr('string'),
  createdBy: DS.belongsTo('user'),
  createdAt: DS.attr('date'),
  updatedAt: DS.attr('date'),
  bytes: DS.attr('number'),
  awsEtag: DS.attr('string'),
  awsPartNumber: DS.attr('string'),
  pending: DS.attr('boolean'),
  file: DS.belongsTo('file')
});
