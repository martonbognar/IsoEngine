#include "isoengine/render/window.h"
#include "isoengine/render/sprite.h"
#include "isoengine/render/text.h"
#include <SFML/Window/Event.hpp>

namespace iso::render
{

sf::Transform toSfmlTransform(const iso::math::Transform & transform)
{
    const auto & m = transform.getMatrix();
    return sf::Transform(m(0, 0), m(0, 1), m(0, 2),
                         m(1, 0), m(1, 1), m(1, 2),
                         m(2, 0), m(2, 1), m(2, 2));
}

namespace
{

constexpr int windowStyleToSfStyle(WindowStyle style)
{
    switch (style) {
    case WindowStyle::Resize:
        return sf::Style::Resize | sf::Style::Close;
    case WindowStyle::Static:
        return sf::Style::Close;
    case WindowStyle::Fullscreen:
        return sf::Style::Fullscreen;
    default:
        return -1; // TODO: throw an exception?
    }
}

} // namespace

Window::Window(const std::string & title, const iso::math::Vector2u & size, WindowStyle style) : window(sf::VideoMode(size.x, size.y), title, windowStyleToSfStyle(style))
{
}

iso::math::Vector2u Window::getSize() const
{
    return {window.getSize().x, window.getSize().y};
}

void Window::setSize(const iso::math::Vector2u & size)
{
    window.setSize({size.x, size.y});
}

bool Window::isOpen() const
{
    return window.isOpen();
}

void Window::clear(sf::Color color)
{
    window.clear(color);
}

void Window::close()
{
    window.close();
}

bool Window::pollEvent(sf::Event & event)
{
    return window.pollEvent(event);
}

void Window::setView(sf::View view)
{
    window.setView(view);
}

const sf::View & Window::getView()
{
    return window.getView();
}

void Window::display()
{
    window.display();
}

void Window::draw(const Sprite & sprite, iso::math::Transform transform)
{
    sf::RenderStates states;
    states.transform = toSfmlTransform(transform);
    window.draw(sprite.getSprite(), states);
}

void Window::draw(const Text & text, iso::math::Transform transform)
{
    sf::RenderStates states;
    states.transform = toSfmlTransform(transform);
    window.draw(text.getText(), states);
}

} // namespace iso
