#pragma once
#include <QObject>


namespace ldb::gui {

  // Forward declaration needed here
  class TracerPanel;

  /**
   * @brief Base class of all views that should updateView their content when the
   * tracer is updated.
   */
  class TracerView {
  public:
    TracerView(TracerPanel* parent) : tracer_panel(parent) {}

    virtual ~TracerView() = default;

  protected:
    TracerPanel* tracer_panel;

  private:
  };

}// namespace ldb::gui
