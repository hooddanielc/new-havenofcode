'use strict';

module.exports = {
  up: function (queryInterface, Sequelize) {
    return queryInterface.createTable('article', {
      id: {
        type: Sequelize.INTEGER,
        primaryKey: true,
        autoIncrement: true
      },
      createdAt: {
        type: Sequelize.DATE
      },
      updatedAt: {
        type: Sequelize.DATE
      },
      user: {
        type: Sequelize.INTEGER,
        references: { model: 'user', key: 'id' },
        allowNull: false
      },
      title: {
        type: Sequelize.STRING,
        unique: true,
        allowNull: false
      },
      description: {
        type: Sequelize.STRING,
        allowNull: false
      },
      md: {
        type: Sequelize.TEXT,
        allowNull: false
      },
      published: {
        type: Sequelize.BOOLEAN,
        allowNull: false,
        defaultValue: false
      }
    });
  },

  down: function (queryInterface, Sequelize) {
    return queryInterface.dropTable('article');
  }
};
