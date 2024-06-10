// splitHtmlTextIntoTwoEqualColumnsTrait.php
<?php

/**
 * TCPDF doesn't support to have a 2 columns text where the length of the text is limited and the height of the 2 columns are equal.
 *
 * This trait calculates the middle of the text, split it into 2 parts and returns with them
 * Keeps the word boundaries and takes care of the HTML tags too! There is no broken HTML tag after the split.
 */
trait splitHtmlTextIntoTwoEqualColumnsTrait
{
    protected function splitHtmlTextIntoTwoEqualColumns(string $htmlText): array
    {
        // removes unnecessary characters and HTML tags
        $htmlText = str_replace("\xc2\xa0", ' ', $htmlText);
        $htmlText = html_entity_decode($htmlText);
        $pureText = $this->getPureText($htmlText);

        // calculates the length of the text
        $fullLength = strlen($pureText);
        $halfLength = ceil($fullLength / 2);

        $words = explode(' ', $pureText);

        // finds the word which is in the middle of the text
        $middleWordPosition = $this->getPositionOfMiddleWord($words, $halfLength);

        // iterates through the HTML and split the text into 2 parts when it reaches the middle word.
        $columns = $this->splitHtmlStringInto2Strings($htmlText, $middleWordPosition);

        return $this->closeUnclosedHtmlTags($columns, $halfLength * 2);
    }

    private function getPureText(string $htmlText): string
    {
        $pureText = strip_tags($htmlText);
        $pureText = preg_replace('/[\x00-\x1F\x7F]/', '', $pureText);

        return str_replace(["\r\n", "\r", "\n"], ['', '', ''], $pureText);
    }

    /**
     * finds the word which is in the middle of the text
     */
    private function getPositionOfMiddleWord(array $words, int $halfLength): int
    {
        $wordPosition = 0;
        $stringLength = 0;
        for ($p = 0; $p < count($words); $p++) {
            $stringLength += mb_strlen($words[$p], 'UTF-8') + 1;
            if ($stringLength > $halfLength) {
                $wordPosition = $p;
                break;
            }
        }

        return $wordPosition;
    }

    /**
     * iterates through the HTML and split the text into 2 parts when it reaches the middle word.
     */
    private function splitHtmlStringInto2Strings(string $htmlText, int $wordPosition): array
    {
        $columns = [
            1 => '',
            2 => '',
        ];

        $columnId    = 1;
        $wordCounter = 0;
        $inHtmlTag   = false;
        for ($s = 0; $s <= strlen($htmlText) - 1; $s++) {
            if ($inHtmlTag === false && $htmlText[$s] === '<') {
                $inHtmlTag = true;
            }

            if ($inHtmlTag === true) {
                $columns[$columnId] .= $htmlText[$s];
                if ($htmlText[$s] === '>') {
                    $inHtmlTag = false;
                }
            } else {
                if ($htmlText[$s] === ' ') {
                    $wordCounter++;
                }
                if ($wordCounter > $wordPosition && $columnId < 2) {
                    $columnId++;
                    $wordCounter = 0;
                }

                $columns[$columnId] .= $htmlText[$s];
            }
        }

        return array_map('trim', $columns);
    }

    private function closeUnclosedHtmlTags(array $columns, int $maxLength): array
    {
        $column1      = $columns[1];
        $unclosedTags = $this->getUnclosedHtmlTags($columns[1], $maxLength);
        foreach (array_reverse($unclosedTags) as $tag) {
            $column1 .= '</' . $tag . '>';
        }

        $column2 = '';
        foreach ($unclosedTags as $tag) {
            $column2 .= '<' . $tag . '>';
        }
        $column2 .= $columns[2];

        return [$column1, $column2];
    }

    /**
     * https://stackoverflow.com/a/26175271/5356216
     */
    private function getUnclosedHtmlTags(string $html, int $maxLength = 250): array
    {
        $htmlLength = strlen($html);
        $unclosed   = [];
        $counter    = 0;
        $i          = 0;
        while (($i < $htmlLength) && ($counter < $maxLength)) {
            if ($html[$i] == "<") {
                $currentTag = "";
                $i++;
                if (($i < $htmlLength) && ($html[$i] != "/")) {
                    while (($i < $htmlLength) && ($html[$i] != ">") && ($html[$i] != "/")) {
                        $currentTag .= $html[$i];
                        $i++;
                    }
                    if ($html[$i] == "/") {
                        do {
                            $i++;
                        } while (($i < $htmlLength) && ($html[$i] != ">"));
                    } else {
                        $currentTag = explode(" ", $currentTag);
                        $unclosed[] = $currentTag[0];
                    }
                } elseif ($html[$i] == "/") {
                    array_pop($unclosed);
                    do {
                        $i++;
                    } while (($i < $htmlLength) && ($html[$i] != ">"));
                }
            } else {
                $counter++;
            }
            $i++;
        }
        return $unclosed;
    }
}
