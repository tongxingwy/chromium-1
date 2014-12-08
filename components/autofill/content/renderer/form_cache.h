// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CONTENT_RENDERER_FORM_CACHE_H_
#define COMPONENTS_AUTOFILL_CONTENT_RENDERER_FORM_CACHE_H_

#include <map>
#include <set>
#include <vector>

#include "base/strings/string16.h"

namespace blink {
class WebDocument;
class WebElement;
class WebElementCollection;
class WebFormControlElement;
class WebFrame;
class WebInputElement;
class WebSelectElement;
}

namespace autofill {

struct FormData;
struct FormDataPredictions;

// Manages the forms in a RenderView.
class FormCache {
 public:
  FormCache();
  ~FormCache();

  // Get all form control elements from |elements| that are not part of a form.
  // If |fieldsets| is not NULL, also append the fieldsets encountered that are
  // not part of a form.
  // Exposed for sharing with tests.
  static std::vector<blink::WebFormControlElement>
      GetUnownedAutofillableFormFieldElements(
          const blink::WebElementCollection& elements,
          std::vector<blink::WebElement>* fieldsets);

  // Scans the DOM in |frame| extracting and storing forms that have not been
  // seen before. Returns the extracted forms.
  std::vector<FormData> ExtractNewForms(const blink::WebFrame& frame);

  // Resets the forms for the specified |frame|.
  void ResetFrame(const blink::WebFrame& frame);

  // Clears the values of all input elements in the form that contains
  // |element|.  Returns false if the form is not found.
  bool ClearFormWithElement(const blink::WebFormControlElement& element);

  // For each field in the |form|, sets the field's placeholder text to the
  // field's overall predicted type.  Also sets the title to include the field's
  // heuristic type, server type, and signature; as well as the form's signature
  // and the experiment id for the server predictions.
  bool ShowPredictions(const FormDataPredictions& form);

 private:
  // Scans |control_elements| and returns the number of editable elements.
  // Also remembers the initial <select> and <input> element states, and
  // logs warning messages for deprecated attribute if
  // |log_deprecation_messages| is set.
  size_t ScanFormControlElements(
      const std::vector<blink::WebFormControlElement>& control_elements,
      bool log_deprecation_messages);

  // The cached web frames and their corresponding synthetic FormData.
  // The synthetic FormData is for all the fieldsets in the document without a
  // form owner.
  std::map<blink::WebDocument, FormData> documents_to_synthetic_form_map_;

  // The cached forms per frame. Used to prevent re-extraction of forms.
  std::map<const blink::WebFrame*, std::set<FormData> > parsed_forms_;

  // The cached initial values for <select> elements.
  std::map<const blink::WebSelectElement, base::string16>
      initial_select_values_;

  // The cached initial values for checkable <input> elements.
  std::map<const blink::WebInputElement, bool> initial_checked_state_;

  DISALLOW_COPY_AND_ASSIGN(FormCache);
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CONTENT_RENDERER_FORM_CACHE_H_
