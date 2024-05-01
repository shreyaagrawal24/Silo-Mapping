#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QScreen>
#include <QStyle>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for the MainWindow class
     * @param parent The parent widget
     */
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

signals:
    /**
     * @brief Signal emitted when a silo widget is clicked
     * @param silo The silo widget
     */
    void siloClicked(QWidget* silo);

private slots:
    /**
     * @brief Slot to handle the click event of a silo widget
     * @param silo The silo widget
     */
    void onSiloClicked(QWidget* silo);

    /**
     * @brief Slot to handle the click event of a recipe button
     */
    void onRecipeClicked();

    /**
     * @brief Slot to handle the clear recipe button click
     */
    void onClearRecipeBtnClicked();

    /**
     * @brief Slot to handle the remove silo button click
     */
    void onRemoveBtnClicked();

    /**
     * @brief Slot to handle the add silo button click
     */
    void onAddBtnClicked();

    /**
     * @brief Slot to handle the reset button click
     */
    void onResetBtnClicked();

    /**
     * @brief Slot to handle the save button click
     */
    void onSaveBtnClicked();

private:
    /**
     * @brief Loads JSON data from a specified file path
     * @param jsonFilePath1 The path to the JSON file 1
     * @param jsonFilePath2 The path to the JSON file 2
     */
    void loadJSONData(const std::string &jsonFilePath1, const std::string &jsonFilePath2);

    /**
     * @brief Loads JSON data to a specified file path
     * @param jsonFilePath The path to the JSON file
     */
    void updateJSONData(const std::string& jsonFilePath);

    /**
     * @brief Filters events for the event filter object
     * @param obj The object that is being watched for events
     * @param event The event to be filtered
     * @return true if the event was filtered, false otherwise
     */
    bool eventFilter(QObject* obj, QEvent* event);

    /**
     * @brief Displays the silo widgets in the silo grid layout
     */
    void displaySilos();

    /**
     * @brief Sets up a silo widget with the provided key and value
     * @param key The key associated with the silo
     * @param value The value associated with the silo
     * @return The constructed QWidget representing the silo
     */
    QWidget* setSilo(const std::string &key, const std::string &value);

    /**
     * @brief Displays recipes in the recipes vertical layout
     */
    void displayRecipes();

    Ui::MainWindow *ui; /**< Pointer to the main window UI object */
    QSize windowSize; /**< Size of the main window */
    json sanjivaniJsonObject, recipeSelectionJsonObject; /**< JSON objects containing data for processing */
    QWidget *lastClickedSilo; /**< Pointer to the last clicked silo widget */
    std::vector<QWidget*> silos; /**< Vector storing pointers to silo widgets */
    std::vector<QPushButton*> recipes; /**< Vector storing pointers to recipe buttons */
    QString clickedSiloRecipe; /**< Recipe associated with the last clicked silo */
    bool initialDisplay = true; /**< Flag used for initial display through the JSON file */
};

#endif // MAINWINDOW_H
