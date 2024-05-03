#include "mainwindow.h"
#include "ui_mainwindow.h"

//JSON file paths
#define SanjivaniJSON "./config/sanjivani.json"
#define RecipeJSON "./config/recipeSelection.json"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    loadJSONData(SanjivaniJSON, RecipeJSON);

    ui->setupUi(this);

    windowSize = QGuiApplication::screens().at(0)->size();

    ui->clearRecipeBtn->setEnabled(false);
    ui->removeBtn->setEnabled(false);

    initialDisplay = true;

    displaySilos();
    displayRecipes();

    connect(this, &MainWindow::siloClicked, this, &MainWindow::onSiloClicked);
    connect(ui->clearRecipeBtn, &QPushButton::clicked, this, &MainWindow::onClearRecipeBtnClicked);
    connect(ui->removeBtn, &QPushButton::clicked, this, &MainWindow::onRemoveBtnClicked);
    connect(ui->addBtn, &QPushButton::clicked, this, &MainWindow::onAddBtnClicked);
    connect(ui->resetBtn, &QPushButton::clicked, this, &MainWindow::onResetBtnClicked);
    connect(ui->saveBtn, &QPushButton::clicked, this, &MainWindow::onSaveBtnClicked);
}

MainWindow::~MainWindow()
{
    delete ui;

    for(QWidget* silo : silos)
    {
        delete silo;
    }

    for(QPushButton* recipe : recipes)
    {
        delete recipe;
    }
}

void MainWindow::loadJSONData(const std::string &jsonFilePath1, const std::string &jsonFilePath2)
{
    std::ifstream sanjivaniFile(jsonFilePath1);
    if(!sanjivaniFile.is_open())
    {
        qDebug() << "Error opening JSON file 1";
        return;
    }
    std::ifstream recipeSelectionFile(jsonFilePath2);
    if(!recipeSelectionFile.is_open())
    {
        qDebug() << "Error opening JSON file 2";
        return;
    }

    try
    {
        sanjivaniJsonObject = json::parse(sanjivaniFile);
        recipeSelectionJsonObject = json::parse(recipeSelectionFile);
    }
    catch (const json::type_error& e)
    {
        qDebug() << "Type Error: " << e.what();
        return;
    }
    catch(const json::parse_error& e)
    {
        qDebug() << "Parse Error: " << e.what();
        return;
    }
    catch(const std::out_of_range& e)
    {
        qDebug() << "Out of range error: " << e.what();
        return;
    }
    catch(const std::exception& e)
    {
        qDebug() << "Error parsing JSON file" << e.what();
        return;
    }
}

void MainWindow::updateJSONData(const std::string &jsonFilePath)
{
    std::ifstream inputSanjivaniFile(jsonFilePath);
    if(!inputSanjivaniFile.is_open())
    {
        qDebug() << "Error opening input JSON file";
        return;
    }

    json jsonObject;
    try
    {
        inputSanjivaniFile >> jsonObject;
        inputSanjivaniFile.close();
        jsonObject.at("sanjivaniSiloMap").clear();

        for(std::vector<QWidget*>::size_type i = 0; i < silos.size(); ++i)
        {
            QWidget* siloWidget = silos[i];
            QLabel* recipeLabel = siloWidget->findChild<QLabel*>("recipeLabel"); //To access the recipe label of the silo widget

            if(recipeLabel)
            {
                std::string recipeName = recipeLabel->text().trimmed().toStdString();
                std::string siloName = std::to_string(i + 1);;
                jsonObject.at("sanjivaniSiloMap")[siloName] = recipeName;
            }
        }

        std::ofstream outputSanjivaniFile(jsonFilePath);
        if(!outputSanjivaniFile.is_open())
        {
            qDebug() << "Error opening output JSON file";
            return;
        }

        outputSanjivaniFile << std::setw(4) << jsonObject << std::endl;
        outputSanjivaniFile.close();
    }
    catch (const json::type_error& e)
    {
        qDebug() << "Type Error: " << e.what();
        return;
    }
    catch(const json::parse_error& e)
    {
        qDebug() << "Parse Error: " << e.what();
        return;
    }
    catch(const std::out_of_range& e)
    {
        qDebug() << "Out of range error: " << e.what();
        return;
    }
    catch (const std::exception &e)
    {
        qDebug() << "Error parsing JSON file:" << e.what();
        return;
    }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        QWidget *clickedSilo = qobject_cast<QWidget*>(obj);
        if(clickedSilo && clickedSilo->isEnabled())
        {
            lastClickedSilo = clickedSilo;
            emit siloClicked(clickedSilo);
            return true;
        }
    }
    return false;
}

void MainWindow::displaySilos()
{
    ui->silosLayout->setAlignment(Qt::AlignTop);
    ui->silosLayout->setSpacing(40); //spacing of the silos grid layout
    ushort row = 0;
    ushort col = 0;
    int silosPerRow = (((windowSize.width() - 40) * 0.75) - 20) / ((windowSize.width() / 6) + 20) ; //Here, 40 is sum of left and right padding padding

    if(initialDisplay)
    {
        try
        {
            for(auto pair = sanjivaniJsonObject.at("sanjivaniSiloMap").begin(); pair != sanjivaniJsonObject.at("sanjivaniSiloMap").end(); ++pair)
            {
                QWidget *silo = setSilo(pair.key(), pair.value());
                silos.push_back(silo);
                ui->silosLayout->addWidget(silo, row, col);
                ++col;
                if(col == silosPerRow)
                {
                    col = 0;
                    ++row;
                }
            }
        }
        catch(const std::out_of_range& e)
        {
            qDebug() << "Out of range error: " << e.what();
            return;
        }
        catch (const std::exception &e)
        {
            qDebug() << "Error parsing JSON file:" << e.what();
            return;
        }
        initialDisplay = false;
    }

    else
    {
        for (QWidget *silo : silos)
        {
            ui->silosLayout->addWidget(silo, row, col);

            ++col;
            if (col == silosPerRow)
            {
                col = 0;
                ++row;
            }
        }
    }

    ui->totalSilosValue->setText(QString::number(ui->silosLayout->count()));
}

QWidget* MainWindow::setSilo(const std::string &key, const std::string &value)
{
    QWidget *silo = new QWidget(this);
    silo->setFixedSize(windowSize.width() / 6, windowSize.height() / 5);
    if(value.empty() || QString::fromStdString(value).trimmed() == "None")
    {
        silo->setProperty("silo", "empty");
        silo->style()->unpolish(silo);
        silo->style()->polish(silo);
        silo->update();
    }
    else
    {
        silo->setProperty("silo", "filled");
        silo->style()->unpolish(silo);
        silo->style()->polish(silo);
        silo->update();
    }
    silo->setObjectName(QString::fromStdString(key));

    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(silo);
    effect->setBlurRadius(5);
    effect->setXOffset(0);
    effect->setYOffset(5);
    effect->setColor(Qt::gray);
    silo->setGraphicsEffect(effect);

    QLabel *siloLabel = new QLabel(silo); //silo label of the silo widget
    siloLabel->setText("Silo " + QString::fromStdString(key));
    siloLabel->setAlignment(Qt::AlignHCenter);
    siloLabel->setObjectName("siloLabel");

    QSize siloSize = silo->size();
    QLabel *siloImageLabel = new QLabel(silo); //image label of the silo widget
    siloImageLabel->setPixmap(QPixmap(":/img/silo.png").scaled(QSize(siloSize.width() / 3, siloSize.height() / 3), Qt::KeepAspectRatio));
    siloImageLabel->setAlignment(Qt::AlignCenter);
    siloImageLabel->setObjectName("siloImageLabel");

    QLabel *recipeLabel = new QLabel(silo); //recipe label of the silo widget
    recipeLabel->setText(QString::fromStdString(value) + "   ");
    recipeLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    recipeLabel->setObjectName("recipeLabel");

    QVBoxLayout *siloLayout = new QVBoxLayout(silo);
    siloLayout->addWidget(siloLabel);
    siloLayout->addWidget(siloImageLabel);
    siloLayout->addWidget(recipeLabel);
    silo->setLayout(siloLayout);
    silo->installEventFilter(this);

    return silo;
}

void MainWindow::onSiloClicked(QWidget* clickedSilo)
{
    clickedSilo->setProperty("silo", "clicked");
    clickedSilo->style()->unpolish(clickedSilo);
    clickedSilo->style()->polish(clickedSilo);
    clickedSilo->update();

    ui->clearRecipeBtn->setEnabled(true);
    ui->removeBtn->setEnabled(true);
    ui->addBtn->setEnabled(false);
    for(QWidget *silo:silos)
    {
        silo->setEnabled(false);
    }

    QLabel *clickedLabel = clickedSilo->findChild<QLabel*>("recipeLabel"); //To access the recipe label of the last clicked silo widget
    clickedSiloRecipe = clickedLabel->text();
    if(clickedLabel)
    {
        clickedLabel->setText("None   ");
    }

    for(QPushButton *recipeButton : recipes)
    {
        connect(recipeButton, &QPushButton::clicked, this, &MainWindow::onRecipeClicked);
    }
}

void MainWindow::displayRecipes()
{
    QLayoutItem *child;
    while((child = ui->recipeListLayout->takeAt(0)) != nullptr)
    {
        delete child;
    }

    try
    {
        for(auto recipe = recipeSelectionJsonObject.at("Recipes").begin(); recipe != recipeSelectionJsonObject.at("Recipes").end(); ++recipe)
        {
            QPushButton *recipeButton = new QPushButton(this); //recipe button
            recipes.push_back(recipeButton);
            recipeButton->setMinimumHeight(50);

            QLabel *recipeLabel2 = new QLabel(recipeButton); //recipe label of the recipe button
            recipeLabel2->setObjectName("recipeLabel2");
            recipeLabel2->setText(QString::fromStdString(recipe.key()));

            bool connectedToSilo = false;
            std::string silo = "None";
            for(QWidget *siloWidget : silos)
            {
                QLabel* siloLabel = siloWidget->findChild<QLabel*>("recipeLabel"); //To access the recipe label of the silo widget
                if(siloLabel && siloLabel->text().trimmed().toStdString() == recipe.key())
                {
                    connectedToSilo = true;
                    silo = siloWidget->objectName().toStdString();
                    break;
                }
                if (siloLabel && recipeSelectionJsonObject.at("Recipes").find(siloLabel->text().trimmed().toStdString()) == recipeSelectionJsonObject.at("Recipes").end())
                {
                    siloLabel->setText("None   ");
                    siloWidget->setProperty("silo", "empty");
                    siloWidget->style()->unpolish(siloWidget);
                    siloWidget->style()->polish(siloWidget);
                    siloWidget->update();
                }
            }

            QLabel *siloLabel2 = new QLabel(recipeButton); //silo label of the recipe button
            siloLabel2->setObjectName("siloLabel2");
            siloLabel2->setAlignment(Qt::AlignRight | Qt::AlignBottom);

            if(connectedToSilo)
            {
                recipeButton->setObjectName("connected");
                siloLabel2->setText("Silo " + QString::fromStdString(silo));
            }
            else
            {
                recipeButton->setObjectName("disconnected");
                siloLabel2->setText(QString::fromStdString(silo));
            }

            QHBoxLayout *recipeLayout = new QHBoxLayout(recipeButton);
            recipeLayout->addWidget(recipeLabel2);
            recipeLayout->addWidget(siloLabel2);
            recipeButton->setLayout(recipeLayout);

            ui->recipeListLayout->addWidget(recipeButton);
        }
    }
    catch(const std::out_of_range& e)
    {
        qDebug() << "Out of range error: " << e.what();
        return;
    }
    catch(const std::exception& e)
    {
        qDebug() << "Error parsing JSON file" << e.what();
        return;
    }
    ui->recipeListLayout->addStretch();
}

void MainWindow::onRecipeClicked()
{
    QPushButton *clickedRecipe = qobject_cast<QPushButton*>(sender());
    if(!clickedRecipe || !lastClickedSilo)
        return;

    QLabel *clickedRecipeLabel = clickedRecipe->findChild<QLabel*>("recipeLabel2"); //To access the recipe label of the clicked recipe button
    QString recipeName = clickedRecipeLabel->text();
    QLabel *clickedSiloLabel = lastClickedSilo->findChild<QLabel*>("recipeLabel"); //To access the recipe label of the last clicked silo widget
    std::string clickedSiloName = lastClickedSilo->objectName().toStdString();

    bool alreadyAssigned = false;

    for(QWidget *siloWidget : silos)
    {
        QLabel* siloLabel = siloWidget->findChild<QLabel*>("recipeLabel"); //To access the recipe label of the silo widget
        if(siloWidget != lastClickedSilo && siloLabel && siloLabel->text().trimmed().toStdString() == recipeName.toStdString())
        {
            alreadyAssigned = true;
            break;
        }
    }

    if (alreadyAssigned)
    {
        QMessageBox::critical(this, "Error", "Recipe already assigned to another silo.");
        clickedSiloLabel->setText(clickedSiloRecipe);
    }
    else
    {
        QLabel* recipeLabel = lastClickedSilo->findChild<QLabel*>("recipeLabel"); //To access the recipe label of the last clicked silo widget
        if(recipeLabel)
        {
            recipeLabel->setText(recipeName + "   ");
        }
    }

    ui->clearRecipeBtn->setEnabled(false);
    ui->removeBtn->setEnabled(false);
    ui->addBtn->setEnabled(true);
    for(QWidget* silo : silos)
    {
        silo->setEnabled(true);
    }

    for(QPushButton* recipeButton : recipes)
    {
        disconnect(recipeButton, &QPushButton::clicked, this, &MainWindow::onRecipeClicked);
    }

    if(clickedSiloLabel->text().trimmed().isEmpty() || clickedSiloLabel->text().trimmed() == "None")
    {
        lastClickedSilo->setProperty("silo", "empty");
        lastClickedSilo->style()->unpolish(lastClickedSilo);
        lastClickedSilo->style()->polish(lastClickedSilo);
        lastClickedSilo->update();
    }
    else
    {
        lastClickedSilo->setProperty("silo", "filled");
        lastClickedSilo->style()->unpolish(lastClickedSilo);
        lastClickedSilo->style()->polish(lastClickedSilo);
        lastClickedSilo->update();
    }

    lastClickedSilo = nullptr;
    displayRecipes();
}

void MainWindow::onClearRecipeBtnClicked()
{
    ui->clearRecipeBtn->setEnabled(false);
    ui->removeBtn->setEnabled(false);
    ui->addBtn->setEnabled(true);
    if(!lastClickedSilo)
    {
        return;
    }
    QLabel* recipeLabel = lastClickedSilo->findChild<QLabel*>("recipeLabel"); //To access the recipe label of the last clicked silo widget
    if(recipeLabel)
    {
        recipeLabel->setText("");
    }
    displayRecipes();
    for(QWidget* silo: silos)
    {
        silo->setEnabled(true);
    }

    lastClickedSilo->setProperty("silo", "empty");
    lastClickedSilo->style()->unpolish(lastClickedSilo);
    lastClickedSilo->style()->polish(lastClickedSilo);
    lastClickedSilo->update();
    lastClickedSilo = nullptr;
}

void MainWindow::onRemoveBtnClicked()
{
    ui->clearRecipeBtn->setEnabled(false);
    ui->removeBtn->setEnabled(false);
    ui->addBtn->setEnabled(true);

    if(lastClickedSilo)
    {
        auto it = std::find(silos.begin(), silos.end(), lastClickedSilo);
        if(it != silos.end())
        {
            silos.erase(it);
            lastClickedSilo->setGraphicsEffect(nullptr); //This line will remove the shadow of a silo when it is deleted
            ui->silosLayout->removeWidget(lastClickedSilo);
            lastClickedSilo->deleteLater();
            for(std::vector<QWidget*>::size_type i = 0; i < silos.size(); ++i)
            {
                QWidget* siloWidget = silos[i];
                siloWidget->setObjectName(QString::fromStdString(std::to_string(i + 1)));
                QLabel* siloLabel = siloWidget->findChild<QLabel*>("siloLabel"); //To access the silo label of the silo widget
                if(siloLabel)
                {
                    siloLabel->setText("Silo " + QString::number(i + 1));
                }
            }
        }
        displaySilos();
        displayRecipes();
    }

    for(QWidget* silo:silos)
    {
        silo->setEnabled(true);
    }

    lastClickedSilo = nullptr;
}

void MainWindow::onAddBtnClicked()
{
    if(ui->silosLayout->count() >= ui->recipeListLayout->count() - 1)
    {
        QMessageBox::critical(this, "Error", "Number of silos can't be greater than number of recipes available.");
        return;
    }
    std::string newSiloName = std::to_string(ui->silosLayout->count() + 1);
    QWidget *newSilo = setSilo(newSiloName, "None   ");
    silos.push_back(newSilo);
    displaySilos();
}

void MainWindow::onResetBtnClicked()
{
    for (QWidget* silo : silos)
    {
        silo->setGraphicsEffect(nullptr);
        ui->silosLayout->removeWidget(silo);
        silo->deleteLater();
    }
    silos.clear();

    for (QPushButton *recipeButton : recipes)
    {
            delete recipeButton;
    }
    recipes.clear();

    initialDisplay = true;
    displaySilos();
    displayRecipes();
    for(QWidget* silo : silos)
    {
        silo->setEnabled(true);
    }
    lastClickedSilo = nullptr;
}

void MainWindow::onSaveBtnClicked()
{
    QMessageBox::information(this, "Congrats", "The JSON file has been updated!");
    updateJSONData(SanjivaniJSON);
}
