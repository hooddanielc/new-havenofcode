'use strict';

module.exports = {
  up: function (queryInterface, Sequelize) {
    return queryInterface.createTable('registration', {
      id: {
        type: Sequelize.INTEGER,
        primaryKey: true,
        autoIncrement: true
      },
      createdAt: {
        type: Sequelize.DATE,
        defaultValue: Sequelize.literal('NOW()')
      },
      updatedAt: {
        type: Sequelize.DATE,
        defaultValue: Sequelize.literal('NOW()')
      },
      deleted: {
        type: Sequelize.BOOLEAN,
        defaultValue: false
      },
      user: {
        type: Sequelize.INTEGER,
        references: { model: 'user', key: 'id' },
        allowNull: false
      },
      rsaPubD: {
        type: Sequelize.STRING,
        allowNull: false
      },
      rsaPubN: {
        type: Sequelize.STRING,
        allowNull: false
      },
      secret: {
        type: Sequelize.STRING,
        allowNull: false
      }
    });
  },

  down: function (queryInterface, Sequelize) {
    return queryInterface.dropTable('registration');
  }
};
