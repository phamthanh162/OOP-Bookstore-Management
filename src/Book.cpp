#include "Book.h"

// Cài đặt Constructor mặc định
Book::Book() : quantity(0), price(0.0) {}

// Cài đặt Constructor có tham số
Book::Book(QString id, QString title, QString author, int quantity, double price)
    : id(id), title(title), author(author), quantity(quantity), price(price) {}

// Cài đặt Getters
QString Book::getId() const { return id; }
QString Book::getTitle() const { return title; }
QString Book::getAuthor() const { return author; }
int Book::getQuantity() const { return quantity; }
double Book::getPrice() const { return price; }

// Cài đặt Setters
void Book::setQuantity(int newQuantity) { quantity = newQuantity; }
void Book::setPrice(double newPrice) { price = newPrice; }